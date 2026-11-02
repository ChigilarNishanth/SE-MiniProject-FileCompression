#include "compression_engine.hpp"
#include "archive_io.hpp"
#include "huffman_tree.hpp"
#include "bit_stream.hpp"
#include <fstream>
#include <iostream>

bool CompressionEngine::compress(const std::string& input_path, const std::string& output_path) {
    std::ifstream in_file(input_path, std::ios::binary);
    if (!in_file.is_open()) {
        std::cerr << "Error: Cannot open source file " << input_path << std::endl;
        return false;
    }

    FrequencyTable freq_table;
    uint64_t original_size = 0;
    char byte;

    while (in_file.get(byte)) {
        freq_table.counts[static_cast<uint8_t>(byte)]++;
        original_size++;
    }

    in_file.clear();
    in_file.seekg(0, std::ios::beg);

    std::ofstream out_file(output_path, std::ios::binary);
    if (!out_file.is_open()) {
        std::cerr << "Error: Cannot create archive file " << output_path << std::endl;
        return false;
    }

    if (!ArchiveIO::write_header(out_file, original_size, freq_table)) {
        std::cerr << "Error: Failed to write archive header." << std::endl;
        return false;
    }

    if (original_size == 0) {
        std::cout << "[US-07] Compressed empty file successfully." << std::endl;
        return true;
    }

    auto root = HuffmanTree::build(freq_table);
    std::string codes[256];
    HuffmanTree::generate_codes(root, "", codes);

    BitWriter writer(out_file);
    while (in_file.get(byte)) {
        uint8_t ubyte = static_cast<uint8_t>(byte);
        writer.write_code(codes[ubyte]);
    }

    writer.flush();
    out_file.close();
    in_file.close();

    std::cout << "[US-07] Compression complete: " << input_path 
              << " -> " << output_path 
              << " (" << original_size << " bytes)" << std::endl;

    return true;
}