#include "freq_analyzer.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Assertion failed: " << message << std::endl;
        std::exit(1);
    }
}

void test_empty_file(const std::string& temp_path) {
    {
        std::ofstream out(temp_path, std::ios::binary | std::ios::trunc);
    }

    FrequencyTable table;
    uint64_t total_bytes = 999;
    bool success = FrequencyAnalyzer::analyze_file(temp_path, table, total_bytes);

    expect(success, "analyze_file should succeed on empty file");
    expect(total_bytes == 0, "Empty file total_bytes should be 0");
    for (int i = 0; i < 256; ++i) {
        expect(table.counts[i] == 0, "All counts should be 0 for empty file");
    }

    std::filesystem::remove(temp_path);
}

void test_all_256_bytes(const std::string& temp_path) {
    // Write every byte 0x00 through 0xFF exactly 7 times
    const size_t repeats_per_byte = 7;
    {
        std::ofstream out(temp_path, std::ios::binary | std::ios::trunc);
        std::vector<char> buffer;
        buffer.reserve(256 * repeats_per_byte);
        for (size_t rep = 0; rep < repeats_per_byte; ++rep) {
            for (int b = 0; b < 256; ++b) {
                buffer.push_back(static_cast<char>(static_cast<uint8_t>(b)));
            }
        }
        out.write(buffer.data(), buffer.size());
    }

    FrequencyTable table;
    uint64_t total_bytes = 0;
    bool success = FrequencyAnalyzer::analyze_file(temp_path, table, total_bytes);

    expect(success, "analyze_file should succeed for 256-byte distribution");
    expect(total_bytes == 256 * repeats_per_byte, "Total bytes should equal 256 * repeats_per_byte");
    for (int i = 0; i < 256; ++i) {
        expect(table.counts[i] == repeats_per_byte, "Count for byte " + std::to_string(i) + " should be exactly repeats_per_byte");
    }

    std::filesystem::remove(temp_path);
}

void test_binary_mode_rb(const std::string& temp_path) {
    // Tests that binary mode (rb) is respected:
    // Null byte (0x00), CRLF (\r\n), Ctrl+Z (0x1A - Windows EOF), and 0xFF.
    std::vector<uint8_t> test_bytes = {
        0x00, 0x00, 0x00,             // 3 null bytes
        0x0D, 0x0A, 0x0D, 0x0A,       // 2 CRLF pairs (4 bytes)
        0x1A, 0x1A,                   // 2 Windows EOF markers (must not stop reading)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 5 0xFF bytes
    };

    {
        std::ofstream out(temp_path, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(test_bytes.data()), test_bytes.size());
    }

    FrequencyTable table;
    uint64_t total_bytes = 0;
    bool success = FrequencyAnalyzer::analyze_file(temp_path, table, total_bytes);

    expect(success, "analyze_file should succeed on binary test file");
    expect(total_bytes == test_bytes.size(), "Total bytes must match test_bytes vector size");
    expect(table.counts[0x00] == 3, "0x00 count should be 3");
    expect(table.counts[0x0D] == 2, "0x0D count should be 2");
    expect(table.counts[0x0A] == 2, "0x0A count should be 2");
    expect(table.counts[0x1A] == 2, "0x1A count should be 2");
    expect(table.counts[0xFF] == 5, "0xFF count should be 5");

    // Also test the overload without total_bytes parameter
    FrequencyTable table2;
    bool success2 = FrequencyAnalyzer::analyze_file(temp_path, table2);
    expect(success2, "analyze_file overload should succeed");
    expect(table2.counts[0x00] == 3, "Overload 0x00 count should match");
    expect(table2.counts[0xFF] == 5, "Overload 0xFF count should match");

    std::filesystem::remove(temp_path);
}

void test_unreadable_file_graceful_fail() {
    FrequencyTable table;
    uint64_t total_bytes = 100;
    // Non-existent path
    bool success = FrequencyAnalyzer::analyze_file("/path/that/does/not/exist/at/all.bin", table, total_bytes);
    expect(!success, "Non-existent path should fail gracefully");
    expect(total_bytes == 0, "total_bytes should reset to 0 on failure");

    // Directory path
    bool dir_success = FrequencyAnalyzer::analyze_file(".", table, total_bytes);
    expect(!dir_success, "Directory path should fail gracefully");
}

void test_stream_analysis() {
    std::string data = "ABRACADABRA";
    std::istringstream stream(data);

    FrequencyTable table;
    uint64_t total_bytes = 0;
    bool success = FrequencyAnalyzer::analyze_stream(stream, table, total_bytes);

    expect(success, "analyze_stream should succeed");
    expect(total_bytes == 11, "Total bytes should be 11");
    expect(table.counts['A'] == 5, "Count for 'A' should be 5");
    expect(table.counts['B'] == 2, "Count for 'B' should be 2");
    expect(table.counts['R'] == 2, "Count for 'R' should be 2");
    expect(table.counts['C'] == 1, "Count for 'C' should be 1");
    expect(table.counts['D'] == 1, "Count for 'D' should be 1");
    expect(table.counts['E'] == 0, "Count for 'E' should be 0");
}

int main(int argc, char** argv) {
    std::string temp_prefix = "/tmp/freq_test_";
    if (argc >= 2) {
        temp_prefix = argv[1];
    }

    std::cout << "[US-02] Running FrequencyAnalyzer acceptance tests..." << std::endl;

    test_empty_file(temp_prefix + "empty.bin");
    test_all_256_bytes(temp_prefix + "all256.bin");
    test_binary_mode_rb(temp_prefix + "binary_rb.bin");
    test_unreadable_file_graceful_fail();
    test_stream_analysis();

    std::cout << "PASS: FrequencyAnalyzer - binary mode (rb), 256-element distribution, graceful error handling" << std::endl;
    return 0;
}
