#include "freq_analyzer.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

bool FrequencyAnalyzer::analyze_stream(std::istream& in_stream,
                                      FrequencyTable& out_table,
                                      uint64_t& out_total_bytes) {
    for (size_t i = 0; i < 256; ++i) {
        out_table.counts[i] = 0;
    }
    out_total_bytes = 0;

    constexpr size_t BUFFER_SIZE = 65536;
    char buffer[BUFFER_SIZE];

    while (in_stream.read(buffer, BUFFER_SIZE) || in_stream.gcount() > 0) {
        std::streamsize bytes_read = in_stream.gcount();
        for (std::streamsize i = 0; i < bytes_read; ++i) {
            uint8_t byte_val = static_cast<uint8_t>(buffer[i]);
            out_table.counts[byte_val]++;
            out_total_bytes++;
        }
    }

    if (in_stream.bad()) {
        std::cerr << "Error: I/O failure while reading binary stream." << std::endl;
        return false;
    }

    return true;
}

bool FrequencyAnalyzer::analyze_file(const std::string& input_path,
                                    FrequencyTable& out_table,
                                    uint64_t& out_total_bytes) {
    for (size_t i = 0; i < 256; ++i) {
        out_table.counts[i] = 0;
    }
    out_total_bytes = 0;

    std::error_code ec;
    if (std::filesystem::is_directory(input_path, ec)) {
        std::cerr << "Error: Cannot open '" << input_path << "' (path is a directory)." << std::endl;
        return false;
    }

    std::ifstream in_file(input_path, std::ios::in | std::ios::binary);
    if (!in_file.is_open()) {
        std::cerr << "Error: Cannot open input file '" << input_path << "' in binary mode (file not found or unreadable)." << std::endl;
        return false;
    }

    if (!analyze_stream(in_file, out_table, out_total_bytes)) {
        std::cerr << "Error: Failed while reading binary file '" << input_path << "'." << std::endl;
        return false;
    }

    return true;
}

bool FrequencyAnalyzer::analyze_file(const std::string& input_path,
                                    FrequencyTable& out_table) {
    uint64_t total_bytes = 0;
    return analyze_file(input_path, out_table, total_bytes);
}
