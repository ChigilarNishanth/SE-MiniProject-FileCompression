#include "decompression_engine.hpp"
#include "compression_engine.hpp"
#include "archive_io.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Assertion failed: " << message << std::endl;
        std::exit(1);
    }
}

void test_empty_file(const std::string& base) {
    std::string src = base + "_empty.bin";
    std::string czip = base + "_empty.czip";
    std::string restored = base + "_empty.restored";

    {
        std::ofstream out(src, std::ios::binary | std::ios::trunc);
    }

    expect(CompressionEngine::compress(src, czip), "compress empty file failed");
    expect(DecompressionEngine::decompress(czip, restored), "decompress empty file failed");

    expect(std::filesystem::exists(restored), "restored empty file does not exist");
    expect(std::filesystem::file_size(restored) == 0, "restored file is not 0 bytes");

    std::filesystem::remove(src);
    std::filesystem::remove(czip);
    std::filesystem::remove(restored);
}

void test_single_symbol(const std::string& base) {
    std::string src = base + "_single.bin";
    std::string czip = base + "_single.czip";
    std::string restored = base + "_single.restored";

    std::string data(10001, 'X');
    {
        std::ofstream out(src, std::ios::binary | std::ios::trunc);
        out.write(data.data(), data.size());
    }

    expect(CompressionEngine::compress(src, czip), "compress single symbol failed");
    expect(DecompressionEngine::decompress(czip, restored), "decompress single symbol failed");

    expect(std::filesystem::exists(restored), "restored single file does not exist");
    expect(std::filesystem::file_size(restored) == data.size(), "size mismatch for single symbol");

    std::ifstream in(restored, std::ios::binary);
    std::string restored_data((std::istreambuf_iterator<char>(in)), {});
    expect(restored_data == data, "exact byte mismatch for single symbol");

    std::filesystem::remove(src);
    std::filesystem::remove(czip);
    std::filesystem::remove(restored);
}

void test_all_256_bytes(const std::string& base) {
    std::string src = base + "_all256.bin";
    std::string czip = base + "_all256.czip";
    std::string restored = base + "_all256.restored";

    std::vector<uint8_t> data;
    data.reserve(256 * 20);
    for (int rep = 0; rep < 20; ++rep) {
        for (int b = 0; b < 256; ++b) {
            data.push_back(static_cast<uint8_t>(b));
        }
    }

    {
        std::ofstream out(src, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    expect(CompressionEngine::compress(src, czip), "compress all 256 bytes failed");
    expect(DecompressionEngine::decompress(czip, restored), "decompress all 256 bytes failed");

    expect(std::filesystem::file_size(restored) == data.size(), "size mismatch for 256 bytes");

    std::ifstream in(restored, std::ios::binary);
    std::vector<uint8_t> restored_data((std::istreambuf_iterator<char>(in)), {});
    expect(restored_data == data, "byte-by-byte mismatch for 256 bytes");

    std::filesystem::remove(src);
    std::filesystem::remove(czip);
    std::filesystem::remove(restored);
}

void test_truncated_bitstream_bounds_checking(const std::string& base) {
    // TC-10: SEC-REQ-02 Malformed & Truncated Bitstream Fuzzing Test
    std::string src = base + "_trunc.bin";
    std::string czip = base + "_trunc.czip";
    std::string bad_czip = base + "_bad_trunc.czip";
    std::string restored = base + "_trunc.restored";

    std::string data = "The quick brown fox jumps over the lazy dog. A quick test for bounds safety.";
    for (int i = 0; i < 10; ++i) data += data;

    {
        std::ofstream out(src, std::ios::binary | std::ios::trunc);
        out.write(data.data(), data.size());
    }

    expect(CompressionEngine::compress(src, czip), "compress failed");

    // Read valid archive and truncate payload
    std::ifstream in_czip(czip, std::ios::binary);
    std::vector<char> czip_bytes((std::istreambuf_iterator<char>(in_czip)), {});
    in_czip.close();

    // Truncate the last 20 bytes of payload
    expect(czip_bytes.size() > 50, "archive too small for truncation test");
    czip_bytes.resize(czip_bytes.size() - 20);

    {
        std::ofstream out_bad(bad_czip, std::ios::binary | std::ios::trunc);
        out_bad.write(czip_bytes.data(), czip_bytes.size());
    }

    // Decompression should fail gracefully without crashing
    bool success = DecompressionEngine::decompress(bad_czip, restored);
    expect(!success, "Truncated bitstream should fail gracefully");

    std::filesystem::remove(src);
    std::filesystem::remove(czip);
    std::filesystem::remove(bad_czip);
    std::filesystem::remove(restored);
}

void test_invalid_magic_header(const std::string& base) {
    // TC-09: Invalid header magic bytes
    std::string bad_czip = base + "_bad_magic.czip";
    std::string restored = base + "_bad_magic.restored";

    {
        std::ofstream out_bad(bad_czip, std::ios::binary | std::ios::trunc);
        out_bad.write("NOT_A_VALID_HEADER_FOR_CZIP", 27);
    }

    bool success = DecompressionEngine::decompress(bad_czip, restored);
    expect(!success, "Invalid header magic should fail immediately");

    std::filesystem::remove(bad_czip);
    std::filesystem::remove(restored);
}

void test_unreadable_archive() {
    std::string restored = "/tmp/should_not_exist.bin";
    bool success = DecompressionEngine::decompress("/path/does/not/exist.czip", restored);
    expect(!success, "Nonexistent archive should fail gracefully");
}

int main(int argc, char** argv) {
    std::string temp_prefix = "/tmp/decomp_test";
    if (argc >= 2) {
        temp_prefix = argv[1];
    }

    std::cout << "[US-08] Running DecompressionEngine acceptance tests..." << std::endl;

    test_empty_file(temp_prefix);
    test_single_symbol(temp_prefix);
    test_all_256_bytes(temp_prefix);
    test_truncated_bitstream_bounds_checking(temp_prefix);
    test_invalid_magic_header(temp_prefix);
    test_unreadable_archive();

    std::cout << "PASS: DecompressionEngine - safe bounds traversal, lossless restoration, error handling" << std::endl;
    return 0;
}
