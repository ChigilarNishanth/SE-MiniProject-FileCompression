# Mudit: remaining sprint stories

## US-06: bit packing
Validated byte boundaries, zero padding, repeated flush and destructor flush. Invalid bit values and non-binary code strings are rejected before writing any part of the invalid code.

## US-09: CLI statistics
Successful compression reports actual input/archive byte sizes, percentage savings and elapsed milliseconds using std::chrono. Empty input reports zero savings; archive overhead can produce negative savings. Stream write failures are propagated before reporting success.

## US-10: SHA-256 verification
Six generated fixtures (empty, single symbol, text, every byte value, seeded random binary and PPM image) are compressed with the actual CLI. A clearly labelled test-only restoration oracle verifies SHA-256 and exact bytes. Statistics are verified against actual filesystem sizes and independently calculated savings.

Run the production acceptance gate after US-08 is integrated:

```sh
python3 tests/test_lossless.py build/czip /tmp/reference_restore --production-decoder
```

Currently this gate fails because the team's production -d path validates only the header and creates no restored file. The oracle is not a production decoder and US-10 must not be marked fully accepted until the production gate passes. US-08 belongs to Mahesh; its implementation is not changed here.

## Scope
Only Mudit's Sprint-2 stories (#6, #9 and #10) are included. No Sprint-3 work is included.
