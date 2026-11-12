"""US-09/US-10: actual CLI statistics and SHA-256 verification.

Default mode tests compression with an explicitly test-only restoration oracle.
--production-decoder requires the real CLI -d implementation and never hides a failure.
"""
import argparse
import hashlib
from pathlib import Path
import random
import re
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('executable', type=Path)
parser.add_argument('reference_restore', type=Path, nargs='?')
parser.add_argument('--production-decoder', action='store_true')
args = parser.parse_args()
if not args.production_decoder and args.reference_restore is None:
    parser.error('reference_restore is required unless --production-decoder is used')
exe = args.executable.resolve()
reference = args.reference_restore.resolve() if args.reference_restore else None
fixtures = {'empty': b'', 'one_byte': b'\x00', 'single': b'A' * 10001,
            'text': b'Huffman lossless compression verification.\n' * 500,
            'binary': bytes(range(256)) * 100,
            'random': random.Random(279).randbytes(65536),
            'image': b'P6\n16 16\n255\n' + bytes(range(256)) * 3}
with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    for name, data in fixtures.items():
        source, archive, restored = [root / (name + ext) for ext in ('.bin', '.czip', '.restored')]
        source.write_bytes(data)
        result = subprocess.run([str(exe), '-c', str(source), str(archive)],
                                capture_output=True, text=True, check=True, timeout=30)
        original = int(re.search(r'Original size: (\d+) bytes', result.stdout)[1])
        packed = int(re.search(r'Compressed size: (\d+) bytes', result.stdout)[1])
        savings = float(re.search(r'Savings: (-?[\d.]+)%', result.stdout)[1])
        elapsed = float(re.search(r'Execution time: ([\d.]+) ms', result.stdout)[1])
        assert original == len(data) and packed == archive.stat().st_size
        expected = 100 * (1 - packed / original) if original else 0
        assert abs(savings - expected) <= .006 and elapsed >= 0
        command = ([str(exe), '-d'] if args.production_decoder else [str(reference)])
        subprocess.run(command + [str(archive), str(restored)], check=True, timeout=30,
                       capture_output=True)
        assert restored.exists(), 'Production decoder did not create output (US-08 dependency)'
        recovered = restored.read_bytes()
        assert hashlib.sha256(data).digest() == hashlib.sha256(recovered).digest(), name
        assert data == recovered, name
        print('PASS:', name, 'SHA-256 and exact bytes; sizes/savings/time verified')
print('Decoder mode:', 'PRODUCTION CLI' if args.production_decoder else 'TEST-ONLY ORACLE')
