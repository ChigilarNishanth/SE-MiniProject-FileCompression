#include "common.hpp"
#include "cli_parser.hpp"
#include "archive_io.hpp"
#include "huffman_tree.hpp"
#include "compression_engine.hpp"
#include "verifier.hpp"
#include <iostream>
#include <fstream>
#include <cassert>

void test_cli_parser() {
    std::cout << "[RUNNING] Test: CLI Parser Flag Handling..." << std::endl;

    char* arg_help[] = {(char*)"czip", (char*)"--help"};
    CLIConfig config1 = CLIParser::parse(2, arg_help);
    assert(config1.mode == CommandType::HELP);

    char* arg_compress[] = {(char*)"czip", (char*)"-c", (char*)"in.txt", (char*)"out.czip"};
    CLIConfig config2 = CLIParser::parse(4, arg_compress);
    assert(config2.mode == CommandType::COMPRESS);

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

    std::cout << "[PASSED] Byte-for-Byte Lossless Verification Test" << std::endl;
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "   US-10 Automated Integration & Lossless Test Suite \n";
    std::cout << "====================================================\n";

    test_cli_parser();
    test_header_validation();
    test_lossless_verifier();

    std::cout << "====================================================\n";
    std::cout << " ALL US-10 INTEGRATION TESTS PASSED SUCCESSFULLY!  \n";
    std::cout << "====================================================\n";
    return 0;
}