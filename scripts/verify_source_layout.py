"""Compare ordered source fragments with the original v1.3 C file, byte for byte."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PARTS = ('platform_win32.h','state.inc','text.inc','events.inc','sampling.inc','report.inc','repairs.inc','runtime.inc','ui.inc')
BASE = '0e2afa801ef331d90f2b54f270e4b462ed24b4c9'

def main():
    original = subprocess.run(['git', 'show', f'{BASE}:src/BlazeUltimateOptimizer.c'], cwd=ROOT, capture_output=True, check=True).stdout
    actual = b''.join((ROOT / 'src' / item).read_bytes() for item in PARTS)
    if actual != original:
        raise SystemExit('Source refactor is not byte-for-byte equivalent to v1.3')
    driver = (ROOT / 'src' / 'BlazeUltimateOptimizer.c').read_text()
    includes = [line.split('"')[1] for line in driver.splitlines() if line.startswith('#include "')]
    if includes != list(PARTS):
        raise SystemExit('Driver source omits/reorders modules')
    print(f'PASS: {len(PARTS)} source segments preserve {len(original)} original bytes')

if __name__ == '__main__':
    main()
