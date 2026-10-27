"""Measure release CLI speed and process RSS, without storing large fixtures in Git."""
import hashlib
import json
from pathlib import Path
import platform
import random
import shutil
import subprocess
import sys
import tempfile
import time

exe = Path(sys.argv[1]).resolve()
large_mb = int(sys.argv[2]) if len(sys.argv) > 2 else 500
report = {'platform': platform.platform(), 'architecture': platform.machine(), 'cases': []}
with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    fixtures = [('text', (b'The quick brown fox jumps over the lazy dog. Huffman coding is lossless.\n' * 16000)[:1000000]),
                ('binary', random.Random(27).randbytes(1000000))]
    for label, block in fixtures:
        source, archive, restored = root / 'source', root / 'archive', root / 'restored'
        with source.open('wb') as f:
            for _ in range(large_mb):
                f.write(block)
        metrics = {}
        for mode, inp, out in [('-c', source, archive), ('-d', archive, restored)]:
            command = [str(exe), mode, str(inp), str(out)]
            if platform.system() == 'Darwin':
                command = ['/usr/bin/time', '-l', *command]
            start = time.perf_counter()
            result = subprocess.run(command, capture_output=True, text=True, check=True)
            elapsed = time.perf_counter() - start
            rss_bytes = None
            if platform.system() == 'Darwin':
                for line in result.stderr.splitlines():
                    if 'maximum resident set size' in line:
                        rss_bytes = int(line.split()[0])
            metrics[mode] = {'seconds': elapsed, 'MB_per_second': source.stat().st_size / elapsed / 1000000,
                             'max_rss_bytes': rss_bytes}
        def digest(path):
            h = hashlib.sha256()
            with path.open('rb') as f:
                for chunk in iter(lambda: f.read(1048576), b''):
                    h.update(chunk)
            return h.hexdigest()
        report['cases'].append({'fixture': label, 'input_bytes': source.stat().st_size,
                                'archive_bytes': archive.stat().st_size, 'sha256_match': digest(source) == digest(restored),
                                'metrics': metrics})
        for path in (source, archive, restored):
            path.unlink()
print(json.dumps(report, indent=2))
