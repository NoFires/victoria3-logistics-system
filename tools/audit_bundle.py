"""Audit the pristine sharing folder, excluding no files implicitly.

Do this before creating a virtual environment or private outputs in the folder.
The scanner is a backstop; it cannot prove the meaning of every arbitrary number.
"""
import argparse
import ast
import re
from pathlib import Path

ALLOWED = {'.md', '.py', '.txt'}
PATTERNS = {
    'address-like hexadecimal literal': re.compile(r'\b0[xX][0-9a-fA-F]{5,}\b'),
    'native include directive': re.compile(r'^\s*#\s*include\s*[<"]', re.MULTILINE),
    'native source code fence': re.compile(r'^```(?:cpp|c\+\+|cxx|cc)\s*$', re.MULTILINE),
    'native cast syntax': re.compile(r'\b(?:reinterpret_cast|static_cast|const_cast)\s*<'),
}


def audit(root):
    failures, count = [], 0
    for path in sorted(root.rglob('*')):
        if not path.is_file():
            continue
        count += 1
        relative = str(path.relative_to(root))
        if path.suffix.lower() not in ALLOWED:
            failures.append((relative, 'file type is not allowed'))
            continue
        try:
            text = path.read_text(encoding='utf-8')
        except UnicodeError:
            failures.append((relative, 'not UTF-8 text'))
            continue
        for label, pattern in PATTERNS.items():
            if pattern.search(text):
                failures.append((relative, label))
        if path.suffix == '.py':
            try:
                ast.parse(text)
            except SyntaxError:
                failures.append((relative, 'invalid Python syntax'))
    return count, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path, nargs='?', default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    count, failures = audit(args.folder)
    for path, reason in failures:
        print('FAIL %s: %s' % (path, reason))
    print('%d text files checked; %d violations' % (count, len(failures)))
    return bool(failures)


if __name__ == '__main__':
    raise SystemExit(main())
