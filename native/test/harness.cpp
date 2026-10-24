// Standalone D3D11 harness for the Ironroad loader.
// It loads dist\version.dll by full path, verifies the forwarded version.dll
// exports still work, and presents frames so the Present hook + ImGui backend
// can be exercised without Victoria 3. Pass --nodll to test the harness alone.
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#include <cstring>
#include <string>

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
  if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
  return DefWindowProcW(h, m, w, l);
}

int wmain(int argc, wchar_t** argv) {
  const wchar_t* dllPath = argc > 1 && wcscmp(argv[1], L"--nodll") != 0 ? argv[1] : L"dist\\version.dll";
  const bool noDll = (argc > 1 && wcscmp(argv[1], L"--nodll") == 0) ||
                     (argc > 2 && wcscmp(argv[2], L"--nodll") == 0);

  if (!noDll) {
    HMODULE self = LoadLibraryW(dllPath);
    if (!self) {
      std::printf("FAIL load %ls err=%lu\n", dllPath, GetLastError());
      return 2;
    }
    std::printf("loaded %ls at %p\n", dllPath, self);
    auto proxySize = (DWORD(WINAPI*)(LPCSTR, LPDWORD))GetProcAddress(self, "GetFileVersionInfoSizeA");
    auto proxyInfo = (BOOL(WINAPI*)(LPCSTR, DWORD, DWORD, LPVOID))GetProcAddress(self, "GetFileVersionInfoA");
    auto proxyQuery = (BOOL(WINAPI*)(LPCVOID, LPCSTR, LPVOID*, PUINT))GetProcAddress(self, "VerQueryValueA");
    HMODULE sys = LoadLibraryW(L"C:\\Windows\\System32\\version.dll");
    auto sysSize = (DWORD(WINAPI*)(LPCSTR, LPDWORD))GetProcAddress(sys, "GetFileVersionInfoSizeA");
    char exe[MAX_PATH];
    std::snprintf(exe, sizeof(exe), "%s", "C:\\Windows\\System32\\notepad.exe");
    DWORD h1 = 0, h2 = 0;
    DWORD a = proxySize ? proxySize(exe, &h1) : 0;
    DWORD b = sysSize ? sysSize(exe, &h2) : 0;
    std::printf("export parity GetFileVersionInfoSizeA: proxy=%lu system=%lu\n", a, b);
    if (a != b) return 3;
    if (a) {
      void* buf = malloc(a);
      if (!proxyInfo(exe, h2, a, buf)) { std::printf("FAIL GetFileVersionInfoA\n"); return 4; }
      VS_FIXEDFILEINFO* fi = nullptr;
      UINT len = 0;
      if (!proxyQuery(buf, "\\", (void**)&fi, &len) || !fi) {
        std::printf("FAIL VerQueryValueA\n");
        return 5;
      }
      std::printf("file version %u.%u.%u.%u\n", HIWORD(fi->dwFileVersionMS),
                  LOWORD(fi->dwFileVersionMS), HIWORD(fi->dwFileVersionLS),
                  LOWORD(fi->dwFileVersionLS));
      free(buf);
    }
  }

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"IronroadHarness";
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  RegisterClassExW(&wc);

  // Tall enough to inspect the complete Logistics sidebar without scrolling.
  RECT want{0, 0, 960, 960};
  AdjustWindowRect(&want, WS_OVERLAPPEDWINDOW, FALSE);
  HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Ironroad harness", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, want.right - want.left,
                              want.bottom - want.top, nullptr, nullptr, wc.hInstance, nullptr);
  if (!hwnd) { std::printf("FAIL window\n"); return 6; }
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  RECT client{};
  GetClientRect(hwnd, &client);
  UINT cw = (UINT)(client.right - client.left);
  UINT ch = (UINT)(client.bottom - client.top);
  std::printf("client %ux%u\n", cw, ch);

  DXGI_SWAP_CHAIN_DESC desc{};
  desc.BufferCount = 1;
  desc.BufferDesc.Width = cw;
  desc.BufferDesc.Height = ch;
  desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.OutputWindow = hwnd;
  desc.SampleDesc.Count = 1;
  desc.Windowed = TRUE;
  desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  ID3D11Device* device = nullptr;
  ID3D11DeviceContext* context = nullptr;
  IDXGISwapChain* swap = nullptr;
  D3D_FEATURE_LEVEL level{};
  HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr,
                                             0, D3D11_SDK_VERSION, &desc, &swap, &device, &level,
                                             &context);
  if (FAILED(hr)) { std::printf("FAIL d3d11 0x%08lx\n", (unsigned long)hr); return 7; }
  DXGI_SWAP_CHAIN_DESC got{};
  swap->GetDesc(&got);
  std::printf("swapchain out=%p hwnd=%p buffers=%u %ux%u\n", got.OutputWindow, hwnd,
              got.BufferCount, got.BufferDesc.Width, got.BufferDesc.Height);

  ID3D11RenderTargetView* rtv = nullptr;
  ID3D11Texture2D* back = nullptr;
  swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back);
  device->CreateRenderTargetView(back, nullptr, &rtv);
  back->Release();

  DWORD start = GetTickCount();
  int frames = 0;
  MSG msg;
  while (GetTickCount() - start < 8000) {
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    const float clear[4] = {0.10f, 0.12f, 0.16f, 1.0f};
    context->OMSetRenderTargets(1, &rtv, nullptr);
    context->ClearRenderTargetView(rtv, clear);
    D3D11_VIEWPORT vp{0, 0, (float)cw, (float)ch, 0, 1};
    context->RSSetViewports(1, &vp);
    hr = swap->Present(1, 0);
    if (frames < 3) std::printf("present %d hr=0x%08lx\n", frames, (unsigned long)hr);
    ++frames;
    Sleep(16);
  }
  std::printf("presented %d frames\n", frames);
  rtv->Release();
  swap->Release();
  context->Release();
  device->Release();
  return frames > 100 ? 0 : 8;
}

