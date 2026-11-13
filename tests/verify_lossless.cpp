#include "common.hpp"
#include "cli_parser.hpp"
#include "archive_io.hpp"
#include "huffman_tree.hpp"
#include "bit_stream.hpp"
#include "freq_analyzer.hpp"
#include "compression_engine.hpp"
#include "decompression_engine.hpp"
#include "verifier.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <filesystem>

void test_cli_parser() {
    std::cout << "[RUNNING] Test: CLI Parser Flag Handling..." << std::endl;

    char* arg_help[] = {(char*)"czip", (char*)"--help"};
    CLIConfig config1 = CLIParser::parse(2, arg_help);
    assert(config1.mode == CommandType::HELP);

    char* arg_compress[] = {(char*)"czip", (char*)"-c", (char*)"in.txt", (char*)"out.czip"};
    CLIConfig config2 = CLIParser::parse(4, arg_compress);
    assert(config2.mode == CommandType::COMPRESS);
    assert(config2.input_path == "in.txt");
    assert(config2.output_path == "out.czip");

    char* arg_decompress[] = {(char*)"czip", (char*)"-d", (char*)"in.czip", (char*)"out.txt"};
    CLIConfig config3 = CLIParser::parse(4, arg_decompress);
    assert(config3.mode == CommandType::DECOMPRESS);

    // Edge cases and error branches
    char* arg_invalid_len[] = {(char*)"czip", (char*)"-c"};
    CLIConfig config4 = CLIParser::parse(2, arg_invalid_len);
    assert(config4.mode == CommandType::INVALID);

    char* arg_invalid_flag[] = {(char*)"czip", (char*)"-x", (char*)"in", (char*)"out"};
    CLIConfig config5 = CLIParser::parse(4, arg_invalid_flag);
    assert(config5.mode == CommandType::INVALID);

    char* arg_empty[] = {(char*)"czip"};
    CLIConfig config6 = CLIParser::parse(1, arg_empty);
    assert(config6.mode == CommandType::INVALID);

    CLIParser::print_usage("czip.exe");

    std::cout << "[PASSED] CLI Parser Test" << std::endl;
}

void test_header_validation() {
    std::cout << "[RUNNING] Test: Header Serialization & Security Check..." << std::endl;

    std::string test_archive = "build/test_header.czip";
    FrequencyTable freq;
    freq.counts['X'] = 42;

    {
        std::ofstream out(test_archive, std::ios::binary);
        assert(ArchiveIO::write_header(out, 100, freq));
    }

    {
        std::ifstream in(test_archive, std::ios::binary);
        uint64_t size = 0;
        FrequencyTable read_freq;
        assert(ArchiveIO::read_header(in, size, read_freq));
        assert(size == 100);
        assert(read_freq.counts['X'] == 42);
    }

    // Unopened/Bad stream tests
    {
        std::ofstream bad_out;
        bad_out.setstate(std::ios::badbit);
        assert(!ArchiveIO::write_header(bad_out, 100, freq));
    }
    {
        std::ifstream bad_in;
        bad_in.setstate(std::ios::badbit);
        uint64_t s = 0;
        FrequencyTable rf;
        assert(!ArchiveIO::read_header(bad_in, s, rf));
    }

    // Invalid magic byte signature test
    std::string bad_magic_file = "build/test_bad_magic.czip";
    {
        std::ofstream out(bad_magic_file, std::ios::binary);
        uint16_t fake_magic = 0x9999;
        uint64_t fake_sz = 123;
        out.write(reinterpret_cast<const char*>(&fake_magic), sizeof(fake_magic));
        out.write(reinterpret_cast<const char*>(&fake_sz), sizeof(fake_sz));
        out.write(reinterpret_cast<const char*>(freq.counts), sizeof(freq.counts));
    }
    {
        std::ifstream in(bad_magic_file, std::ios::binary);
        uint64_t s = 0;
        FrequencyTable rf;
        assert(!ArchiveIO::read_header(in, s, rf));
    }
    std::filesystem::remove(bad_magic_file);

    // Truncated stream test
    std::string trunc_file = "build/test_trunc.czip";
    {
        std::ofstream out(trunc_file, std::ios::binary);
        char partial[4] = {'C', 'Z', 1, 0};
        out.write(partial, sizeof(partial));
    }
    {
        std::ifstream in(trunc_file, std::ios::binary);
        uint64_t s = 0;
        FrequencyTable rf;
        assert(!ArchiveIO::read_header(in, s, rf));
    }
    std::filesystem::remove(trunc_file);

    // Truncated frequency table test (valid 10-byte header, truncated counts table)
    std::string trunc_freq_file = "build/test_trunc_freq.czip";
    {
        std::ofstream out(trunc_freq_file, std::ios::binary);
        uint16_t magic = CZIP_MAGIC;
        uint64_t sz = 100;
        out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        out.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
        char partial_counts[64] = {0}; // only 64 bytes instead of 2048
        out.write(partial_counts, sizeof(partial_counts));
    }
    {
        std::ifstream in(trunc_freq_file, std::ios::binary);
        uint64_t s = 0;
        FrequencyTable rf;
        assert(!ArchiveIO::read_header(in, s, rf));
    }
    std::filesystem::remove(trunc_freq_file);

    std::cout << "[PASSED] Header Validation Test" << std::endl;
}

void test_lossless_verifier() {
    std::cout << "[RUNNING] Test: US-10 Byte-for-Byte Lossless Verification..." << std::endl;

    std::string f1_path = "build/orig_sample.txt";
    std::string f2_path = "build/copy_sample.txt";

    {
        std::ofstream out1(f1_path, std::ios::binary);
        std::ofstream out2(f2_path, std::ios::binary);
        out1 << "Byte-level accuracy test for US-10 lossless verification.";
        out2 << "Byte-level accuracy test for US-10 lossless verification.";
    }

    uint64_t mismatch_offset = 0;
    bool match = LosslessVerifier::compare_files(f1_path, f2_path, mismatch_offset);
    assert(match == true);

    // Error branches: missing files
    bool missing1 = LosslessVerifier::compare_files("build/nonexistent1.txt", f2_path, mismatch_offset);
    assert(!missing1);
    bool missing2 = LosslessVerifier::compare_files(f1_path, "build/nonexistent2.txt", mismatch_offset);
    assert(!missing2);

    // Different file sizes
    std::string f3_path = "build/diff_size.txt";
    {
        std::ofstream out3(f3_path, std::ios::binary);
        out3 << "Short";
    }
    bool diff_sz = LosslessVerifier::compare_files(f1_path, f3_path, mismatch_offset);
    assert(!diff_sz);
    std::filesystem::remove(f3_path);

    // Same size with byte mismatch
    std::string f4_path = "build/mismatch.txt";
    {
        std::ofstream out4(f4_path, std::ios::binary);
        out4 << "Byte-level accuracy test for US-10 lossless verificatiXn.";
    }
    bool mismatch_match = LosslessVerifier::compare_files(f1_path, f4_path, mismatch_offset);
    assert(!mismatch_match);
    assert(mismatch_offset > 0);
    std::filesystem::remove(f4_path);

    std::cout << "[PASSED] Byte-for-Byte Lossless Verification Test" << std::endl;
}

void test_bit_stream() {
    std::cout << "[RUNNING] Test: BitStream Packing and Flushing..." << std::endl;

    std::string test_bin = "build/test_bitstream.bin";
    {
        std::ofstream out(test_bin, std::ios::binary);
        BitWriter writer(out);
        writer.write_bit(1);
        writer.write_bit(0);
        writer.write_bit(1);
        writer.write_code("10101"); // exactly 8 bits total now
        writer.write_code("11");
        writer.flush(); // flushes 2 bits left-shifted
        writer.flush(); // redundant flush (bit_count == 0 branch)
    }
    std::filesystem::remove(test_bin);

    // Invalid bit exception branch
    {
        std::string test_bin2 = "build/test_bitstream2.bin";
        std::ofstream out2(test_bin2, std::ios::binary);
        BitWriter writer(out2);
        bool caught_bit = false;
        try {
            writer.write_bit(2);
        } catch (const std::invalid_argument&) {
            caught_bit = true;
        }
        assert(caught_bit);

        bool caught_code = false;
        try {
            writer.write_code("10201");
        } catch (const std::invalid_argument&) {
            caught_code = true;
        }
        assert(caught_code);
    }
    std::filesystem::remove("build/test_bitstream2.bin");

    std::cout << "[PASSED] BitStream Test" << std::endl;
}

void test_huffman_tree() {
    std::cout << "[RUNNING] Test: Huffman Tree Construction & Code Generation..." << std::endl;

    // Normal multi-symbol
    FrequencyTable freq;
    freq.counts['A'] = 5;
    freq.counts['B'] = 9;
    freq.counts['C'] = 12;
    freq.counts['D'] = 13;
    freq.counts['E'] = 16;
    freq.counts['F'] = 45;

    auto root = HuffmanTree::build(freq);
    assert(root != nullptr);

    std::string codes[256];
    HuffmanTree::generate_codes(root, "", codes);
    assert(!codes['A'].empty());
    assert(!codes['F'].empty());

    // Single-symbol tree (pq.size() == 1 branch)
    FrequencyTable single_freq;
    single_freq.counts['Z'] = 100;
    auto single_root = HuffmanTree::build(single_freq);
    assert(single_root != nullptr);
    std::string single_codes[256];
    HuffmanTree::generate_codes(single_root, "", single_codes);
    assert(!single_codes['Z'].empty());

    // Empty frequency table (pq.empty() branch)
    FrequencyTable empty_freq;
    auto empty_root = HuffmanTree::build(empty_freq);
    assert(empty_root == nullptr);
    std::string empty_codes[256];
    HuffmanTree::generate_codes(empty_root, "", empty_codes); // handles null root branch

    std::cout << "[PASSED] Huffman Tree Test" << std::endl;
}

void test_freq_analyzer() {
    std::cout << "[RUNNING] Test: Frequency Analyzer Stream & File Operations..." << std::endl;

    std::string test_file = "build/freq_sample.txt";
    {
        std::ofstream out(test_file, std::ios::binary);
        out << "Hello Huffman Frequency Analyzer";
    }

    FrequencyTable table;
    uint64_t total = 0;
    assert(FrequencyAnalyzer::analyze_file(test_file, table, total));
    assert(total == 32);
    assert(table.counts['H'] == 2);
    assert(table.counts['e'] == 4);

    // Overload without total parameter
    FrequencyTable table2;
    assert(FrequencyAnalyzer::analyze_file(test_file, table2));
    assert(table2.counts['H'] == 2);

    // Nonexistent file branch
    FrequencyTable bad_table;
    assert(!FrequencyAnalyzer::analyze_file("build/nonexistent_freq.txt", bad_table));

    // Directory path branch
    assert(!FrequencyAnalyzer::analyze_file("build", bad_table));

    // Empty file branch
    std::string empty_file = "build/freq_empty.txt";
    {
        std::ofstream out(empty_file, std::ios::binary);
    }
    assert(FrequencyAnalyzer::analyze_file(empty_file, table, total));
    assert(total == 0);
    std::filesystem::remove(empty_file);
    std::filesystem::remove(test_file);

    // Bad input stream error branch
    {
        std::stringstream bad_stream;
        bad_stream.setstate(std::ios::badbit);
        FrequencyTable st_table;
        uint64_t st_total = 0;
        assert(!FrequencyAnalyzer::analyze_stream(bad_stream, st_table, st_total));
    }

    std::cout << "[PASSED] Frequency Analyzer Test" << std::endl;
}

void test_compression_decompression() {
    std::cout << "[RUNNING] Test: Compression & Decompression Engines..." << std::endl;

    std::string orig = "build/engine_orig.txt";
    std::string czip = "build/engine_arch.czip";
    std::string rest = "build/engine_rest.txt";

    // Standard file test
    {
        std::ofstream out(orig, std::ios::binary);
        out << "Testing the complete CZIP compression and decompression engine pipeline!";
    }

    assert(CompressionEngine::compress(orig, czip));
    assert(DecompressionEngine::decompress(czip, rest));

    uint64_t offset = 0;
    assert(LosslessVerifier::compare_files(orig, rest, offset));

    // Wrapper CompressionEngine::decompress test
    std::string rest2 = "build/engine_rest2.txt";
    assert(CompressionEngine::decompress(czip, rest2));
    assert(LosslessVerifier::compare_files(orig, rest2, offset));
    std::filesystem::remove(rest2);

    // Empty file compression & decompression
    std::string empty_orig = "build/empty_orig.txt";
    std::string empty_czip = "build/empty_arch.czip";
    std::string empty_rest = "build/empty_rest.txt";
    {
        std::ofstream out(empty_orig, std::ios::binary);
    }
    assert(CompressionEngine::compress(empty_orig, empty_czip));
    assert(DecompressionEngine::decompress(empty_czip, empty_rest));
    assert(std::filesystem::file_size(empty_rest) == 0);
    std::filesystem::remove(empty_orig);
    std::filesystem::remove(empty_czip);
    std::filesystem::remove(empty_rest);

    // Error branches: nonexistent input for compress
    assert(!CompressionEngine::compress("build/does_not_exist.txt", "build/out.czip"));

    // Error branch: unwritable output for compress
    assert(!CompressionEngine::compress(orig, "/invalid_dir_path_should_fail/czip.czip"));

    // Error branch: nonexistent archive for decompress
    assert(!DecompressionEngine::decompress("build/no_such_archive.czip", "build/out.txt"));

    // Error branch: corrupted archive (invalid header)
    std::string bad_czip = "build/corrupt_header.czip";
    {
        std::ofstream out(bad_czip, std::ios::binary);
        out << "NOT_A_VALID_HEADER";
    }
    assert(!DecompressionEngine::decompress(bad_czip, "build/corrupt_out.txt"));
    std::filesystem::remove(bad_czip);

    // Error branch: truncated payload during decompression
    std::string trunc_czip = "build/trunc_payload.czip";
    {
        std::ifstream in(czip, std::ios::binary);
        std::vector<char> bytes((std::istreambuf_iterator<char>(in)), {});
        in.close();
        if (bytes.size() > 20) {
            bytes.resize(bytes.size() - 10); // truncate last 10 bytes
        }
        std::ofstream out(trunc_czip, std::ios::binary);
        out.write(bytes.data(), bytes.size());
    }
    assert(!DecompressionEngine::decompress(trunc_czip, "build/trunc_out.txt"));
    std::filesystem::remove(trunc_czip);

    // Error branch: cannot create restored output file
    assert(!DecompressionEngine::decompress(czip, "/invalid_dir_to_fail/out.txt"));

    // Error branch: all-zero frequency table with non-zero original size (triggers if (!root))
    std::string null_tree_czip = "build/null_tree.czip";
    {
        std::ofstream out(null_tree_czip, std::ios::binary);
        uint16_t magic = CZIP_MAGIC;
        uint64_t fake_sz = 100;
        out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        out.write(reinterpret_cast<const char*>(&fake_sz), sizeof(fake_sz));
        char zero_counts[2048] = {0};
        out.write(zero_counts, sizeof(zero_counts));
        out.write("payload", 7);
    }
    assert(!DecompressionEngine::decompress(null_tree_czip, "build/null_tree_out.txt"));
    std::filesystem::remove(null_tree_czip);

    // Error branch: malformed tree traversal (single symbol tree with bit 1 payload)
    std::string bad_traverse_czip = "build/bad_traverse.czip";
    {
        std::ofstream out(bad_traverse_czip, std::ios::binary);
        uint16_t magic = CZIP_MAGIC;
        uint64_t fake_sz = 10;
        out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        out.write(reinterpret_cast<const char*>(&fake_sz), sizeof(fake_sz));
        FrequencyTable single_ft;
        single_ft.counts['A'] = 10;
        out.write(reinterpret_cast<const char*>(single_ft.counts), sizeof(single_ft.counts));
        uint8_t bad_payload = 0xFF; // All 1s, but single-symbol tree only has left branch (bit 0)
        out.write(reinterpret_cast<const char*>(&bad_payload), 1);
    }
    assert(!DecompressionEngine::decompress(bad_traverse_czip, "build/bad_traverse_out.txt"));
    std::filesystem::remove(bad_traverse_czip);

    std::filesystem::remove(orig);
    std::filesystem::remove(czip);
    std::filesystem::remove(rest);

    std::cout << "[PASSED] Compression & Decompression Engines Test" << std::endl;
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "   US-10 Automated Integration & Lossless Test Suite \n";
    std::cout << "====================================================\n";

    std::filesystem::create_directories("build");

    test_cli_parser();
    test_header_validation();
    test_lossless_verifier();
    test_bit_stream();
    test_huffman_tree();
    test_freq_analyzer();
    test_compression_decompression();

    std::cout << "====================================================\n";
    std::cout << " ALL US-10 INTEGRATION TESTS PASSED SUCCESSFULLY!  \n";
    std::cout << "====================================================\n";
    return 0;
}
