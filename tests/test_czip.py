"""Requirement-driven CLI tests; standard-library only."""
import hashlib
import binascii
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile
import unittest

EXE = Path(sys.argv.pop(1)).resolve()


class Requirements(unittest.TestCase):
    def setUp(self):
        self.sandbox = tempfile.TemporaryDirectory()
        self.root = Path(self.sandbox.name)

    def tearDown(self):
        self.sandbox.cleanup()

    def run_cli(self, *args, code=0):
        result = subprocess.run([str(EXE), *map(str, args)], capture_output=True, timeout=30, cwd=self.root)
        self.assertEqual(result.returncode, code, result.stderr.decode(errors='replace'))
        return result

    def round_trip(self, data):
        source = self.root / 'source.bin'
        source.write_bytes(data)
        archive = self.root / 'data.czip'
        restored = self.root / 'restored.bin'
        result = self.run_cli('-c', source, archive)
        self.run_cli('-d', archive, restored)
        self.assertEqual(hashlib.sha256(data).digest(), hashlib.sha256(restored.read_bytes()).digest())
        return archive, result

    def corrupt(self, data, diagnostic=None):
        archive = self.root / 'bad.czip'
        archive.write_bytes(data)
        target = self.root / 'rejected.bin'
        result = self.run_cli('-d', archive, target, code=1)
        self.assertFalse(target.exists())
        self.assertFalse(list(self.root.glob('.czip-tmp-*')))
        if diagnostic:
            self.assertIn(diagnostic.encode(), result.stderr)

    def test_01_text_round_trip(self):
        self.round_trip(b'Huffman compression with lossless recovery.\n' * 1000)

    def test_02_all_byte_values(self):
        self.round_trip(bytes(range(256)) * 37)

    def test_03_empty(self):
        archive, _ = self.round_trip(b'')
        self.assertEqual(archive.stat().st_size, 2072)

    def test_04_single_symbol(self):
        self.round_trip(b'\xff' * 10001)

    def test_05_incompressible(self):
        self.round_trip(random.Random(27).randbytes(100000))

    def test_06_deterministic_tree_and_frequencies(self):
        data = bytes(range(256)) * 4
        archive, _ = self.round_trip(data)
        other = self.root / 'other.czip'
        self.run_cli('-c', self.root / 'source.bin', other)
        self.assertEqual(archive.read_bytes(), other.read_bytes())
        header = archive.read_bytes()[:2072]
        self.assertEqual(header[:4], b'CZ\x01\x00')
        self.assertEqual(struct.unpack_from('<Q', header, 4)[0], len(data))
        self.assertEqual(struct.unpack_from('<256Q', header, 24), (4,) * 256)

    def test_07_stats(self):
        archive, result = self.round_trip(b'A' * 40000)
        self.assertLess(archive.stat().st_size, 40000)
        for label in (b'Original size:', b'Archive size:', b'Savings:', b'Execution time:'):
            self.assertIn(label, result.stdout)

    def test_08_help_and_invalid_syntax(self):
        self.assertIn(b'Usage:', self.run_cli('--help').stdout)
        for args in [[], ['-x'], ['-c'], ['-d', 'x', '--unknown'], ['-d', 'x', '--max-output', '-1'],
                     ['-c', 'x', '--max-output', '1'], ['-d', 'x', '--max-output', '9' * 50],
                     ['-d', 'x', '--max-output', '1', '--max-output', '2']]:
            self.assertIn(b'Usage:', self.run_cli(*args, code=2).stderr)

    def test_09_missing_source(self):
        self.assertIn(b'IO_ERROR', self.run_cli('-c', 'missing', code=1).stderr)

    def test_10_no_clobber_and_input_alias(self):
        source = self.root / 'source'
        source.write_bytes(b'keep me')
        self.run_cli('-c', source, source, code=1)
        self.assertEqual(source.read_bytes(), b'keep me')
        archive, _ = self.round_trip(b'abc' * 100)
        target = self.root / 'occupied'
        target.write_bytes(b'preserve')
        self.run_cli('-d', archive, target, code=1)
        self.assertEqual(target.read_bytes(), b'preserve')

    def test_11_bad_magic_version_flags_and_short_header(self):
        archive, _ = self.round_trip(b'abba' * 100)
        data = archive.read_bytes()
        for index in (0, 2, 3):
            bad = bytearray(data)
            bad[index] ^= 255
            self.corrupt(bad, 'SEC_ERR_BAD_HEADER')
        for length in (0, 1, 12, 2071):
            self.corrupt(data[:length], 'SEC_ERR_BAD_HEADER')

    def test_12_frequency_and_bitcount_validation(self):
        archive, _ = self.round_trip(b'abc' * 100)
        for offset in (4, 12, 24):
            bad = bytearray(archive.read_bytes())
            struct.pack_into('<Q', bad, offset, 2**64 - 1)
            self.corrupt(bad, 'SEC_ERR_BAD_HEADER')

    def test_13_truncation_and_trailing_data(self):
        archive, _ = self.round_trip(b'abc' * 100)
        data = archive.read_bytes()
        self.corrupt(data[:-1], 'SEC_ERR_MALFORMED_STREAM')
        self.corrupt(data + b'\0', 'SEC_ERR_MALFORMED_STREAM')

    def test_14_checksum_and_payload_corruption(self):
        archive, _ = self.round_trip(b'ab' * 100)
        for offset in (20, 2072):
            bad = bytearray(archive.read_bytes())
            bad[offset] ^= 128
            self.corrupt(bad, 'SEC_ERR_MALFORMED_STREAM')

    def test_15_padding_and_single_symbol_validation(self):
        archive, _ = self.round_trip(b'A' * 9)
        for offset, mask in ((-1, 1), (2072, 128)):
            bad = bytearray(archive.read_bytes())
            bad[offset] ^= mask
            self.corrupt(bad, 'SEC_ERR_MALFORMED_STREAM')

    def test_16_output_resource_limit(self):
        archive, _ = self.round_trip(b'A' * 100)
        target = self.root / 'limited'
        self.run_cli('-d', archive, target, '--max-output', '99', code=1)
        self.assertFalse(target.exists())
        self.run_cli('-d', archive, target, '--max-output', '100')
        self.assertEqual(target.read_bytes(), b'A' * 100)

    def test_17_default_names_and_spaces(self):
        source = self.root / 'a file.txt'
        source.write_bytes(b'example' * 100)
        self.run_cli('-c', source)
        source.rename(self.root / 'saved')
        self.run_cli('-d', str(source) + '.czip')
        self.assertEqual(source.read_bytes(), (self.root / 'saved').read_bytes())

    def test_18_seeded_round_trip_property(self):
        rng = random.Random(1793)
        for i in range(25):
            data = bytes(rng.randrange(rng.choice((2, 16, 256))) for _ in range(rng.randrange(1, 5000)))
            source = self.root / f'case-{i}'
            source.write_bytes(data)
            self.run_cli('-c', source)
            target = self.root / f'restored-{i}'
            self.run_cli('-d', str(source) + '.czip', target)
            self.assertEqual(data, target.read_bytes())

    def test_19_seeded_malformed_archives(self):
        rng = random.Random(903)
        archive, _ = self.round_trip(bytes(range(256)) * 4)
        pristine = archive.read_bytes()
        for _ in range(40):
            bad = bytearray(pristine)
            index = rng.randrange(len(bad))
            bad[index] ^= 1 << rng.randrange(8)
            self.corrupt(bad)

    def test_20_output_io_failure(self):
        source = self.root / 'source'
        source.write_bytes(b'abc')
        self.run_cli('-c', source, self.root / 'missing' / 'output', code=1)
        self.assertFalse(list(self.root.glob('.czip-tmp-*')))

    def test_21_independent_golden_archive(self):
        # Independent known-code oracle: A=0, C=10, B=11 for counts 3/1/2.
        # AAABBC => 000111110, then seven zero padding bits => 1F 00.
        data = b'AAABBC'
        frequencies = [0] * 256
        for byte in data:
            frequencies[byte] += 1
        expected = struct.pack('<2sBBQQI256Q', b'CZ', 1, 0, 6, 9,
                               binascii.crc32(data), *frequencies) + b'\x1f\x00'
        archive, _ = self.round_trip(data)
        self.assertEqual(archive.read_bytes(), expected)
        independent = self.root / 'golden.czip'
        independent.write_bytes(expected)
        restored = self.root / 'golden-restored'
        self.run_cli('-d', independent, restored)
        self.assertEqual(restored.read_bytes(), data)


if __name__ == '__main__':
    unittest.main(verbosity=2)
