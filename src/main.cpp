#include "common.hpp"
#include "cli_parser.hpp"
#include "archive_io.hpp"
#include <iostream>
#include <fstream>

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
        std::cout << "Input Path:  " << config.input_path << std::endl;
        std::cout << "Output Path: " << config.output_path << std::endl;

        FrequencyTable freq_table;
        freq_table.counts['A'] = 100;
        freq_table.counts['B'] = 50;

        std::ofstream out_file(config.output_path, std::ios::binary);
        if (!out_file) {
            std::cerr << "Error: Unable to open output file " << config.output_path << std::endl;
            return EXIT_FAILURE_CODE;
        }

        if (!ArchiveIO::write_header(out_file, 150, freq_table)) {
            return EXIT_FAILURE_CODE;
        }

        std::cout << "[US-04] Successfully wrote header with magic byte 0x435A to " << config.output_path << std::endl;
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
        std::cout << "Sample Frequency: 'A' -> " << freq_table.counts['A'] << std::endl;

        return EXIT_SUCCESS_CODE;
    }

    return EXIT_SUCCESS_CODE;
}