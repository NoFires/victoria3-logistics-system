"""Read a local executable; report discovery status without printing locations.

Optional --private-out writes recipient-local findings for manual ABI review.
That output must never be added to this shareable starter package.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path


def discover(exe):
    from ironpe import PE
    from recovery_core import Recover, AnchorBroken
    pe = PE(str(exe))
    rec = Recover(pe)
    results, failures = {}, {}

    def run(name, operation):
        try:
            results[name] = operation()
        except (AnchorBroken, KeyError, ValueError, IndexError, TypeError) as exc:
            failures[name] = str(exc)

    run('constructor', rec.arrow_object_ctor)
    run('constructor_cross_check', rec.arrow_object_ctor_cross)
    run('constant_buffer_slot', rec.constant_buffer_slot)
    run('set_path', rec.arrow_object_set_path)
    run('ensure_constant_buffer', rec.ensure_constant_buffer)
    run('type_resolver', rec.get_arrow_type)
    run('allocator', rec.new_delete_allocator)
    run('constructor_fields', lambda: rec.ctor_field_offsets(results['constructor'][0]))

    def append():
        slot = results['constant_buffer_slot'][0]
        type_field = results['constructor_fields'][0]
        candidates = set()
        # Derive the bucket-field displacement rather than importing its value.
        for target in slot['slots']:
            extent = pe.extent(target)
            if extent is None:
                continue
            for ins in pe.insns(*extent):
                if ins.mnemonic != 'movsx':
                    continue
                match = re.search(r'word ptr \[rdx \+ (0x[0-9a-f]+|[0-9]+)\]', ins.op_str)
                if match and rec._is_bucket_append(target, type_field, int(match[1], 0)):
                    candidates.add(int(match[1], 0))
        if len(candidates) != 1:
            raise AnchorBroken('bucket field', 'expected one derived candidate; found %d' % len(candidates))
        field = candidates.pop()
        return rec.bucket_append(slot, type_field, field), field

    run('bucket_append', append)
    run('bucket_list', lambda: rec.render_bucket_list(results['bucket_append'][0][0]))
    run('render_pass', lambda: rec.render_pass(results['bucket_list'][0]))
    run('render_device', lambda: rec.render_device(results['ensure_constant_buffer'][0]))
    constructor_agrees = ('constructor' in results and 'constructor_cross_check' in results
                          and results['constructor'][0] == results['constructor_cross_check'][0])
    if not constructor_agrees:
        failures['constructor_agreement'] = 'Independent constructor derivations did not agree or were unavailable.'
    return {
        'format': 'local-research-candidates-v1',
        'executable_sha256': hashlib.sha256(pe.data).hexdigest(),
        'executable_bytes': len(pe.data),
        'discovery_complete': not failures,
        'runtime_safe_to_call': False,
        'note': 'Candidate evidence only. Context layout, ABI, live device, lifecycle and call safety require local verification.',
        'results': results,
        'failures': failures,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--private-out', type=Path)
    args = parser.parse_args()
    if args.private_out and args.private_out.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Keep private discovery output outside the shareable package directory.')
    try:
        report = discover(args.exe)
    except Exception as exc:
        # Never print raw exceptions: disassembler errors can contain locations.
        print('Discovery could not start: %s. Check the executable, dependencies and architecture.' % type(exc).__name__)
        return 2
    for name in sorted(set(report['results']) | set(report['failures'])):
        print('%s: %s' % (name, 'NEEDS_REVIEW' if name in report['failures'] else 'CANDIDATE_FOUND'))
    print('Native calls remain disabled until manual ABI and lifecycle verification.')
    if args.private_out:
        args.private_out.parent.mkdir(parents=True, exist_ok=True)
        args.private_out.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print('Private candidate report written outside the shareable package.')
    return 0 if report['discovery_complete'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
