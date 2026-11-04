#include "common.hpp"
#include "cli_parser.hpp"
#include "archive_io.hpp"
#include "compression_engine.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <iomanip>

int main(int argc, char* argv[]) {
    CLIConfig config = CLIParser::parse(argc, argv);

    if (config.mode == CommandType::HELP) {
        CLIParser::print_usage(argv[0]);
        return EXIT_SUCCESS_CODE;
    }

    if (config.mode == CommandType::INVALID) {
        CLIParser::print_usage(argv[0]);
        return EXIT_SYNTAX_ERROR;
    }

    if (config.mode == CommandType::COMPRESS) {
        std::cout << "[US-01] Mode: COMPRESS" << std::endl;
        const auto start = std::chrono::steady_clock::now();
        if (!CompressionEngine::compress(config.input_path, config.output_path)) {
            return EXIT_FAILURE_CODE;
        }
        std::error_code ec;
        const auto original = std::filesystem::file_size(config.input_path, ec);
        if (ec) { std::cerr << "Error: Cannot read input size\n"; return EXIT_FAILURE_CODE; }
        const auto compressed = std::filesystem::file_size(config.output_path, ec);
        if (ec) { std::cerr << "Error: Cannot read archive size\n"; return EXIT_FAILURE_CODE; }
        const double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        const double savings = original ? 100.0 * (1.0 - static_cast<double>(compressed) / static_cast<double>(original)) : 0.0;
        std::cout << "Original size: " << original << " bytes\n"
                  << "Compressed size: " << compressed << " bytes\n"
                  << std::fixed << std::setprecision(2)
                  << "Savings: " << savings << "%\n"
                  << "Execution time: " << elapsed << " ms\n";
        return EXIT_SUCCESS_CODE;
    }

    if (config.mode == CommandType::DECOMPRESS) {
        std::cout << "[US-01] Mode: DECOMPRESS" << std::endl;
        std::cout << "Archive Path:  " << config.input_path << std::endl;
        std::cout << "Restored Path: " << config.output_path << std::endl;

        std::ifstream in_file(config.input_path, std::ios::binary);
        if (!in_file) {
            std::cerr << "Error: Unable to open archive file " << config.input_path << std::endl;
            return EXIT_FAILURE_CODE;
        }

        uint64_t original_size = 0;
        FrequencyTable freq_table;

        if (!ArchiveIO::read_header(in_file, original_size, freq_table)) {
            return EXIT_FAILURE_CODE;
        }

        std::cout << "[US-04] Successfully validated magic header 0x435A!" << std::endl;
        std::cout << "Original File Size: " << original_size << " bytes" << std::endl;

        return EXIT_SUCCESS_CODE;
    }

    return EXIT_SUCCESS_CODE;
}