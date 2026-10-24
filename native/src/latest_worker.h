#pragma once
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include "mutex.h"

namespace v3 {
// All times are milliseconds. Keeping this independent of the wall clock lets
// tests cover sustained churn and the maximum wait without timing guesses.
struct SettleGate {
  bool pending = false;
  uint64_t first = 0, last = 0;
  void Change(uint64_t now) { if (!pending) first = now; last = now; pending = true; }
  DWORD Remaining(uint64_t now) const {
    if (!pending) return INFINITE;
    const auto quiet = now - last, total = now - first;
    if (quiet >= 100 || total >= 250) return 0;
    return static_cast<DWORD>(std::min<uint64_t>(100 - quiet, 250 - total));
  }
};

// One owned request and one immutable completed result. Computation never runs
// under the mailbox lock. Render consumers only take a shared_ptr; they never
// wait for a graph search, join a thread, or wait on a simulation lock.
template<class Input, class Output> class LatestWorker {
 public:
  using Cancelled = std::function<bool()>;
  using Compute = std::function<Output(const Input&, const Cancelled&)>;
  struct Reply { uint64_t generation; Output value; std::string error; };
  explicit LatestWorker(Compute compute) : m_compute(std::move(compute)) {
    m_wake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (m_wake) m_thread = CreateThread(nullptr, 0, &RunThunk, this, 0, nullptr);
    if (!m_thread) {
      if (m_wake) CloseHandle(m_wake);
      throw std::runtime_error("could not start route worker");
    }
  }
  // Normal host shutdown/tests only. Never destroy a worker under loader lock.
  ~LatestWorker() { Stop(); }
  LatestWorker(const LatestWorker&) = delete;
  LatestWorker& operator=(const LatestWorker&) = delete;
  uint64_t Submit(Input input) {
    LockGuard lock(m_mutex);
    const uint64_t generation = ++m_generation;
    m_input = std::move(input);
    m_gate.Change(GetTickCount64());
    m_reply.reset();
    SetEvent(m_wake);
    return generation;
  }
  uint64_t Cancel() {
    LockGuard lock(m_mutex);
    const uint64_t generation = ++m_generation;
    m_input.reset(); m_reply.reset(); m_gate = {};
    SetEvent(m_wake);
    return generation;
  }
  uint64_t Generation() const { return m_generation.load(); }
  std::shared_ptr<const Reply> Poll() {
    LockGuard lock(m_mutex);
    return m_reply;
  }
  void Stop() {
    if (!m_thread) return;
    m_stopping = true; ++m_generation; SetEvent(m_wake);
    WaitForSingleObject(m_thread, INFINITE);
    CloseHandle(m_thread); CloseHandle(m_wake);
    m_thread = m_wake = nullptr;
  }
 private:
  static DWORD WINAPI RunThunk(void* self) {
    static_cast<LatestWorker*>(self)->Run(); return 0;
  }
  void Run() {
    while (!m_stopping.load()) {
      std::optional<Input> input;
      uint64_t generation = 0;
      DWORD wait = INFINITE;
      {
        LockGuard lock(m_mutex);
        wait = m_gate.Remaining(GetTickCount64());
        if (wait == 0) {
          input = std::move(m_input); m_input.reset(); m_gate = {};
          generation = m_generation.load();
        }
      }
      if (!input) { WaitForSingleObject(m_wake, wait); continue; }
      const Cancelled cancelled = [this, generation] {
        return m_stopping.load() || generation != m_generation.load();
      };
      try {
        auto result = std::make_shared<Reply>(Reply{generation, m_compute(*input, cancelled), {}});
        LockGuard lock(m_mutex);
        if (!cancelled()) m_reply = std::move(result);
      } catch (const std::exception& exception) {
        auto result = std::make_shared<Reply>(Reply{generation, {}, exception.what()});
        LockGuard lock(m_mutex);
        if (!cancelled()) m_reply = std::move(result);
      } catch (...) {
        // Do not propagate C++ exceptions across a Win32 thread entry. Compute
        // adapters return a diagnostic rather than leaving the UI waiting for
        // a generation that can no longer complete.
        auto result = std::make_shared<Reply>(Reply{generation, {}, "background calculation failed"});
        LockGuard lock(m_mutex);
        if (!cancelled()) m_reply = std::move(result);
      }
    }
  }
  Compute m_compute;
  Mutex m_mutex;
  HANDLE m_wake = nullptr, m_thread = nullptr;
  std::atomic<bool> m_stopping{false};
  std::atomic<uint64_t> m_generation{0};
  SettleGate m_gate;
  std::optional<Input> m_input;
  std::shared_ptr<const Reply> m_reply;
};
} // namespace v3
