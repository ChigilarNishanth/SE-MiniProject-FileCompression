# Mudit: remaining sprint stories

## US-06: bit packing
Validated byte boundaries, zero padding, repeated flush and destructor flush. Invalid bit values and non-binary code strings are rejected before writing any part of the invalid code.

## US-09: CLI statistics
Successful compression reports actual input/archive byte sizes, percentage savings and elapsed milliseconds using std::chrono. Empty input reports zero savings; archive overhead can produce negative savings. Stream write failures are propagated before reporting success.

## US-10: SHA-256 verification
The original six fixtures verified SHA-256 and exact bytes using a test-only restoration oracle while US-08 was in progress. US-08 is now merged. The current production acceptance suite uses the CLI for both compression and decompression, includes a one-byte file, and checks SHA-256, exact bytes, sizes, savings and timing.

```sh
python3 tests/test_lossless.py build/czip --production-decoder
```

See the [Part 3 unit test report](Unit_Test_Report.md) for the verified results.

## Scope
Only Mudit's Sprint-2 stories (#6, #9 and #10) are included. No Sprint-3 work is included.
