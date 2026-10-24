"""Compile our shader bodies with D3DCompiler, stubbing engine include bindings.

Checks HLSL syntax/types for every palette. Engine include/resource binding and
actual rendering still require the game's own shader compiler at next launch.
"""
import ctypes as c
from pathlib import Path
import re


def blob_method(blob, index, restype, *argtypes):
    table = c.cast(blob, c.POINTER(c.POINTER(c.c_void_p))).contents
    return c.WINFUNCTYPE(restype, c.c_void_p, *argtypes)(table[index])


def compile_hlsl(source, target):
    dll = c.WinDLL('d3dcompiler_47.dll')
    fn = dll.D3DCompile
    fn.argtypes = [c.c_void_p, c.c_size_t, c.c_char_p, c.c_void_p, c.c_void_p,
                   c.c_char_p, c.c_char_p, c.c_uint, c.c_uint,
                   c.POINTER(c.c_void_p), c.POINTER(c.c_void_p)]
    fn.restype = c.c_long
    code, errors = c.c_void_p(), c.c_void_p()
    encoded = source.encode()
    hr = fn(encoded, len(encoded), b'building_flow.shader', None, None,
            b'main', target.encode(), 1 << 11, 0, c.byref(code), c.byref(errors))
    message = ''
    if errors:
        ptr = blob_method(errors, 3, c.c_void_p)(errors)
        size = blob_method(errors, 4, c.c_size_t)(errors)
        message = c.string_at(ptr, size).decode(errors='replace')
        blob_method(errors, 2, c.c_ulong)(errors)
    if code:
        blob_method(code, 2, c.c_ulong)(code)
    if hr < 0:
        raise RuntimeError(message)


def main():
    shader = (Path(__file__).resolve().parents[1] / 'native/assets/building_flow.shader').read_text()
    bodies = re.findall(r'Code\s*\[\[(.*?)\]\]', shader, re.S)
    assert len(bodies) == 2
    # The outer Jomini/Clausewitz DSL uses # comments, while HLSL uses //.
    # D3DCompile alone cannot catch invalid comments outside Code blocks.
    outer = re.sub(r'Code\s*\[\[.*?\]\]', '', shader, flags=re.S)
    assert not re.search(r'^\s*//', outer, re.M), 'Use # comments outside HLSL Code blocks'
    declarations = '''
struct VS_INPUT { float3 vPosition: POSITION; float2 vTexCoord: TEXCOORD0; };
struct VS_OUTPUT { float4 vPosition: SV_POSITION; float2 vTexCoord: TEXCOORD0; };
cbuffer NativeArrow : register(b0) {
    float AnimationLength; float MoveLength; float ClipLength; float TotalLength; float Width;
};
cbuffer NativeFlags : register(b1) { int IsSelected; int IsFaded; };
cbuffer EngineStub : register(b2) { float4x4 ViewProjectionMatrix; float _FlatmapHeight; float _FlatmapLerp; float GlobalTime; };
float4 FixProjectionAndMul(float4x4 m, float4 p) { return mul(m,p); }
float GetHeight(float2 p) { return 0; }
Texture2D DiffuseTexture: register(t0); Texture2D MaskTexture: register(t3);
SamplerState LinearSampler: register(s0);
#define PdxTex2D(t,uv) t.Sample(LinearSampler,uv)
#define PdxTex2DGrad(t,uv,dx,dy) t.SampleGrad(LinearSampler,uv,dx,dy)
'''
    for variant in ('OUTPUT', 'INPUT', 'YELLOW', 'BLUE', 'TRAFFIC_GREEN', 'TRAFFIC_LIME',
                    'TRAFFIC_YELLOW', 'TRAFFIC_ORANGE', 'TRAFFIC_RED'):
        prefix = declarations + f'\n#define IRONROAD_{variant}\n'
        vertex = bodies[0].replace('PDX_MAIN', 'VS_OUTPUT main(VS_INPUT Input)')
        pixel = bodies[1].replace('PDX_MAIN', 'float4 main(VS_OUTPUT Input): SV_TARGET')
        compile_hlsl(prefix + vertex, 'vs_5_0')
        compile_hlsl(prefix + pixel, 'ps_5_0')
    print('PASS: building-flow vertex/pixel HLSL compiles in all nine palettes (engine bindings stubbed).')


if __name__ == '__main__':
    main()
