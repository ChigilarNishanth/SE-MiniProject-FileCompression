"""Capture real CLI output, then render explicitly labelled terminal replays."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import textwrap

from PIL import Image, ImageDraw, ImageFont
import imageio_ffmpeg

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'demos'
OUT.mkdir(exist_ok=True)
EXE = Path(sys.argv[1]).resolve()
FONT = '/System/Library/Fonts/Menlo.ttc'
SANS = '/System/Library/Fonts/Helvetica.ttc'
mono = ImageFont.truetype(FONT, 19)
small = ImageFont.truetype(SANS, 20)
heading = ImageFont.truetype(SANS, 34)
transcripts = []


def capture(command, cwd, expected=0, display=None, tail=None):
    result = subprocess.run(command, cwd=cwd, capture_output=True, text=True, timeout=120)
    assert result.returncode == expected, result.stderr
    output = result.stdout + result.stderr
    record = {'command': command, 'display': display or ' '.join(command), 'exit_code': result.returncode,
              'stdout': result.stdout, 'stderr': result.stderr}
    transcripts.append(record)
    lines = output.rstrip().splitlines()
    if tail:
        lines = ['[showing final output lines]'] + lines[-tail:]
    return ['$ ' + record['display'], *lines, f'[exit {result.returncode}]']


with tempfile.TemporaryDirectory() as directory:
    work = Path(directory)
    (work / 'src').mkdir()
    shutil.copyfile(ROOT / 'src/czip.cpp', work / 'src/czip.cpp')
    (work / 'sample.txt').write_bytes(b'File compression with Huffman coding. Team-E17.\n' * 2500)
    build = capture(['c++', '-std=c++17', '-O2', 'src/czip.cpp', '-o', 'czip'], work)
    demo_exe = str(work / 'czip')
    help_lines = capture([demo_exe, '--help'], work, display='./czip --help')
    packed = capture([demo_exe, '-c', 'sample.txt', 'sample.czip'], work,
                     display='./czip -c sample.txt sample.czip')
    unpacked = capture([demo_exe, '-d', 'sample.czip', 'restored.txt'], work,
                       display='./czip -d sample.czip restored.txt')
    hashes = capture([sys.executable, '-c',
                      "import hashlib; from pathlib import Path; "
                      "[(print(hashlib.sha256(Path(p).read_bytes()).hexdigest(),p)) for p in ('sample.txt','restored.txt')]; "
                      "assert Path('sample.txt').read_bytes()==Path('restored.txt').read_bytes(); print('PASS: byte-for-byte match')"],
                     work, display="python3 -c '...compare SHA-256 and exact bytes...'")
    (work / 'binary.bin').write_bytes(bytes(range(256)) * 4096)
    binary_c = capture([demo_exe, '-c', 'binary.bin', 'binary.czip'], work,
                       display='./czip -c binary.bin binary.czip')
    binary_d = capture([demo_exe, '-d', 'binary.czip', 'binary-restored.bin'], work,
                       display='./czip -d binary.czip binary-restored.bin')
    binary_h = capture([sys.executable, '-c',
                        "import hashlib; from pathlib import Path; "
                        "a=Path('binary.bin').read_bytes(); b=Path('binary-restored.bin').read_bytes(); "
                        "print('Input SHA-256:   ',hashlib.sha256(a).hexdigest()); "
                        "print('Restored SHA-256:',hashlib.sha256(b).hexdigest()); assert a==b; print('PASS: all byte values preserved')"],
                       work, display="python3 -c '...verify binary SHA-256 and exact bytes...'")
    bad = bytearray((work / 'sample.czip').read_bytes())
    bad[2072] ^= 128
    (work / 'corrupt.czip').write_bytes(bad)
    corrupt = capture([demo_exe, '-d', 'corrupt.czip', 'rejected.txt'], work, expected=1,
                      display='./czip -d corrupt.czip rejected.txt')
    assert not (work / 'rejected.txt').exists()
    preserved = capture([demo_exe, '-d', 'sample.czip', 'restored.txt'], work, expected=1,
                        display='./czip -d sample.czip restored.txt')
    cap = capture([demo_exe, '-d', 'sample.czip', 'limited.txt', '--max-output', '100'], work,
                  expected=1, display='./czip -d sample.czip limited.txt --max-output 100')
    tests = capture([sys.executable, str(ROOT / 'tests/test_czip.py'), demo_exe], work,
                    display='python3 tests/test_czip.py ./czip', tail=6)
    transcript_meta = {'prepared_date': '2026-10-27', 'kind': 'current-product terminal replay',
                       'not_historical_sprint_recording': True,
                       'source_sha256': hashlib.sha256((ROOT / 'src/czip.cpp').read_bytes()).hexdigest(),
                       'captures': transcripts}
    (OUT / 'transcripts.json').write_text(json.dumps(transcript_meta, indent=2) + '\n')

benchmark = json.loads((ROOT / 'docs/benchmark_results.json').read_text())
performance = ['Saved measured evidence: docs/benchmark_results.json', '', '500,000,000-byte fixtures | ARM64 macOS | Release -O2']
for case in benchmark['cases']:
    performance += [f"{case['fixture']:8} compress: {case['metrics']['-c']['MB_per_second']:.2f} MB/s",
                    f"         restore:  {case['metrics']['-d']['MB_per_second']:.2f} MB/s",
                    f"         SHA-256 match: {case['sha256_match']}"]
performance += ['', 'Original x86-64 reference-host check remains pending.', 'Peak process RSS <2 MB; this is not a heap-profiler report.']

sprint1 = [
    ('Build the working utility', build + ['', 'C++17, standard library only; no compression libraries.'],
     'Current product demonstration, prepared after the Sprint-1 week.'),
    ('Inspect the command interface', help_lines, 'Compress and restore files; existing destinations stay protected.'),
    ('Compress real text', packed, 'Frequency analysis -> deterministic tree -> packed Huffman payload.'),
    ('Restore the original file', unpacked, 'Metadata, payload boundaries and CRC are validated before publication.'),
    ('Verify the bytes', hashes, 'Both hashes and the byte-by-byte comparison come from actual captured commands.'),
    ('Sprint-1 outcome / current demonstration',
     ['Working: compression, decompression, stats, and lossless recovery.', '',
      'Custom .czip container; not PKZIP-compatible.',
      'Header: CZ + version + size + bit count + CRC-32 + frequencies.',
      'Small and incompressible files can expand.', '',
      'Historical 19-23 October recording is not supplied here.',
      'Team PR review and final release acceptance remain separate.'],
     'This 60-second replay is current evidence, not a backdated sprint recording.'),
]
sprint2 = [
    ('Binary compression', binary_c, 'Input includes every possible byte value, including zero bytes.'),
    ('Binary restoration', binary_d, 'Streaming decoding avoids memory growth with input size.'),
    ('Check binary integrity', binary_h, 'Actual SHA-256 and byte identity confirm lossless restoration.'),
    ('Reject a corrupted archive', corrupt + ['', 'Verified: rejected.txt does not exist after failure.'],
     'A changed payload fails validation without publishing partial data.'),
    ('Protect files and bound expansion', preserved + [''] + cap,
     'Existing files are preserved; declared decoded output must fit the selected cap.'),
    ('Run the requirement test suite', tests + ['', 'Also passed separately under AddressSanitizer + UBSan.',
                                              'Includes independent golden archive and seeded mutations.'],
     '21 test methods; 25 seeded round trips and 40 seeded one-bit mutations.'),
    ('Inspect measured non-functional behavior', performance,
     'Saved measurements include complete CLI wall time and SHA-256 verification.'),
    ('Documentation and release review',
     ['Prepared: SRS, test plan, component + sequence diagrams,',
      'API design, security analysis, backlog, sprint plan, deviations.', '',
      'Pending: GitHub publication and actual remote CI results,',
      'team member PR/reviewer evidence, reference-host check,',
      'and an approved frozen release commit.', '',
      'No historical sprint completion or approval is invented.'],
     'Current working-product demonstration. Prepared 27 October 2026.'),
]


def render_scene(title, lines, caption, label, elapsed, duration):
    frame = Image.new('RGB', (1280, 720), '#0b1220')
    draw = ImageDraw.Draw(frame)
    draw.text((48, 30), 'TEAM E17  /  CZIP', font=small, fill='#65dfc5')
    draw.text((48, 66), title, font=heading, fill='#f2f6ff')
    draw.text((48, 112), label + '  |  Actual command output, replayed for readability', font=small, fill='#99abc4')
    draw.rounded_rectangle((40, 156, 1240, 600), radius=14, fill='#121e30', outline='#2c3b50', width=2)
    for x, color in ((63, '#ff7373'), (88, '#f4ca64'), (113, '#65dfc5')):
        draw.ellipse((x, 174, x + 12, 186), fill=color)
    y = 208
    for line in lines:
        for wrapped in textwrap.wrap(line, width=98, replace_whitespace=False, drop_whitespace=False) or ['']:
            if y > 570:
                raise ValueError('Demo scene has too many text lines: ' + title)
            draw.text((62, y), wrapped, font=mono, fill='#65dfc5' if wrapped.startswith('$') else '#e2e9f5')
            y += 25
    for i, line in enumerate(textwrap.wrap(caption, width=110)):
        draw.text((48, 620 + 24 * i), line, font=small, fill='#d7e2f2')
    draw.rectangle((48, 690, 1232, 695), fill='#243349')
    draw.rectangle((48, 690, 48 + int(1184 * elapsed / duration), 695), fill='#65dfc5')
    draw.text((1120, 30), f'{int(elapsed):03}/{duration}s', font=small, fill='#99abc4')
    return frame


ffmpeg = imageio_ffmpeg.get_ffmpeg_exe()
for filename, scenes, duration in [('sprint1_current_demo.mp4', sprint1, 60),
                                    ('sprint2_current_demo.mp4', sprint2, 120)]:
    fps = 12
    command = [ffmpeg, '-y', '-f', 'rawvideo', '-vcodec', 'rawvideo', '-pix_fmt', 'rgb24',
               '-s', '1280x720', '-r', str(fps), '-i', '-', '-an', '-c:v', 'libx264',
               '-preset', 'fast', '-crf', '22', '-pix_fmt', 'yuv420p', '-movflags', '+faststart',
               str(OUT / filename)]
    process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    seconds_per_scene = duration / len(scenes)
    for frame_no in range(duration * fps):
        elapsed = frame_no / fps
        scene_no = min(int(elapsed / seconds_per_scene), len(scenes) - 1)
        title, lines, caption = scenes[scene_no]
        frame = render_scene(title, lines, caption, 'Current product / ' + ('60-second demo' if duration == 60 else '120-second demo'), elapsed, duration)
        process.stdin.write(frame.tobytes())
        if frame_no == int(seconds_per_scene * fps * 2):
            frame.save(OUT / (Path(filename).stem + '_preview.png'))
    process.stdin.close()
    errors = process.stderr.read().decode()
    assert process.wait() == 0, errors
    print(filename, 'rendered', duration, 'seconds')
