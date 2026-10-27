# Working-product demonstrations

Prepared 27 October 2026 from the actual local executable. These are terminal replays of captured command output with presentation timing, not desktop screen recordings. No terminal output or hash outcome is invented. The videos have readable captions and no audio.

- `sprint1_current_demo.mp4`: exactly 60 seconds; compile/help, compress real text, restore, compare hashes, explain archive and scope.
- `sprint2_current_demo.mp4`: exactly 120 seconds; binary round trip, failure on corrupted payload, preservation of an existing file, test results, benchmarks, and release limitations.
- `transcripts.json`: captured commands, stdout/stderr, exit codes, and SHA-256 checks used by the replay renderer.

The first sprint's required dates were 19-23 October 2026. This package does not relabel a later demonstration as a video recorded during that week. The team must retain or separately explain any missing contemporaneous evidence. Final PR review, GitHub board publication, and release freeze are separate pending steps.

Reproduce with `python3 tools/make_demos.py build/czip` in an environment with Pillow and imageio-ffmpeg. The generator runs real commands in a temporary directory, verifies outcomes, and encodes H.264 video at 1280x720, 12 fps. The source fixtures contain only generated sample data.
