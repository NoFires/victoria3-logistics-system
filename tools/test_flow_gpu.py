"""Run an offscreen GPU regression with the actual shader and installed texture."""
from pathlib import Path
import os
import struct
import subprocess
import tempfile
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
COMPILER = Path('E:/Program/C Compiler/MSYS2/ucrt64/bin/g++.exe')
TEXTURE = Path('C:/Program Files (x86)/Steam/steamapps/common/Victoria 3/game/gfx/lines/arrow_diffuse.dds')


def main():
    output = ROOT / 'native/build/tests/flow_shader_tests.exe'
    output.parent.mkdir(parents=True, exist_ok=True)
    environment = os.environ.copy()
    environment['PATH'] = str(COMPILER.parent) + os.pathsep + environment.get('PATH', '')
    subprocess.run([str(COMPILER), '-std=c++20', '-O2', '-static', '-DNOMINMAX',
                    str(ROOT / 'native/test/flow_shader_tests.cpp'), '-o', str(output),
                    '-ld3d11', '-ld3dcompiler'], check=True, env=environment)
    with tempfile.TemporaryDirectory(prefix='ironroad-flow-gpu-') as directory:
        raw = Path(directory) / 'arrow.rgba'
        texture = Image.open(TEXTURE).convert('RGBA')
        raw.write_bytes(struct.pack('<II', *texture.size) + texture.tobytes())
        subprocess.run([str(output), str(ROOT / 'native/assets/building_flow.shader'), str(raw)], check=True)
        broken = Path(directory) / 'half_width_regression.shader'
        shader = (ROOT / 'native/assets/building_flow.shader').read_text(encoding='utf-8')
        assert 'Input.vTexCoord.x );' in shader
        broken.write_text(shader.replace('Input.vTexCoord.x );', 'Input.vTexCoord.x * 0.5f );'), encoding='utf-8')
        result = subprocess.run([str(output), str(broken), str(raw)], capture_output=True, text=True)
        assert result.returncode != 0 and 'triangle tip must lie on ribbon centre' in result.stdout, result.stdout + result.stderr
        print('PASS: negative control reproduces the clipped, sideways-looking arrowhead and is rejected.')
        assert '"jomini/jomini_water.fxh"' in shader
        assert 'max( GetHeight( pos.xz ), _WaterHeight )' in shader
        submerged = Path(directory) / 'submerged_regression.shader'
        submerged.write_text(shader.replace('max( GetHeight( pos.xz ), _WaterHeight )',
                                            'max( GetHeight( pos.xz ), 0.0f )'), encoding='utf-8')
        result = subprocess.run([str(output), str(submerged), str(raw)], capture_output=True, text=True)
        assert result.returncode != 0 and 'ribbon must stay above the terrain/water surface at every zoom' in result.stdout, result.stdout + result.stderr
        print('PASS: negative control reproduces submerged close-zoom sea routes and is rejected.')
        one_way = Path(directory) / 'one_way_balanced.shader'
        union = 'diffuse = max( diffuse, PdxTex2D( DiffuseTexture, reverseUV ) );'
        assert union in shader
        one_way.write_text(shader.replace(union, 'diffuse = diffuse;'), encoding='utf-8')
        result = subprocess.run([str(output), str(one_way), str(raw)], capture_output=True, text=True)
        assert result.returncode != 0 and 'balanced flow must display both directions' in result.stdout, result.stdout + result.stderr
        print('PASS: negative control rejects a false one-way arrow for equal opposing trade.')


if __name__ == '__main__':
    main()
