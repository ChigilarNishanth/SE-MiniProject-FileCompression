#include "decompression_engine.hpp"
#include "archive_io.hpp"
#include "huffman_tree.hpp"
#include <fstream>
#include <iostream>
#include <vector>

bool DecompressionEngine::decompress(const std::string& archive_path, const std::string& output_path) {
    std::ifstream in_file(archive_path, std::ios::binary);
    if (!in_file.is_open()) {
        std::cerr << "Error: Unable to open archive file " << archive_path << std::endl;
        return false;
    }

    uint64_t original_size = 0;
    FrequencyTable freq_table;

    if (!ArchiveIO::read_header(in_file, original_size, freq_table)) {
        return false;
    }

    std::cout << "[US-04] Successfully validated magic header 0x435A!" << std::endl;
    std::cout << "Original File Size: " << original_size << " bytes" << std::endl;

    std::ofstream out_file(output_path, std::ios::binary | std::ios::trunc);
    if (!out_file.is_open()) {
        std::cerr << "Error: Cannot create restored output file " << output_path << std::endl;
        return false;
    }

    if (original_size == 0) {
        out_file.close();
        std::cout << "[US-08] Decompression complete: restored empty file." << std::endl;
        return true;
    }

    auto root = HuffmanTree::build(freq_table);
    if (!root) {
        std::cerr << "Corrupted Stream: Unable to reconstruct Huffman tree from frequency table." << std::endl;
        out_file.close();
        return false;
    }

    auto node = root;
    uint64_t written = 0;

    constexpr size_t BUFFER_SIZE = 65536;
    char in_buffer[BUFFER_SIZE];
    char out_buffer[BUFFER_SIZE];
    size_t out_buf_pos = 0;

    auto flush_out_buffer = [&]() -> bool {
        if (out_buf_pos > 0) {
            out_file.write(out_buffer, out_buf_pos);
            if (!out_file) {
                std::cerr << "Error: Failed to write restored bytes to " << output_path << std::endl;
                return false;
            }
            out_buf_pos = 0;
        }
        return true;
    };

    while (written < original_size) {
        in_file.read(in_buffer, BUFFER_SIZE);
        std::streamsize bytes_read = in_file.gcount();
        if (bytes_read <= 0) {
            std::cerr << "Corrupted Stream: Unexpected EOF" << std::endl;
            out_file.close();
            return false;
        }

        for (std::streamsize i = 0; i < bytes_read && written < original_size; ++i) {
            uint8_t byte = static_cast<uint8_t>(in_buffer[i]);
            for (int bit = 7; bit >= 0 && written < original_size; --bit) {
                if (!node) {
                    std::cerr << "Corrupted Stream: Malformed tree traversal" << std::endl;
                    out_file.close();
                    return false;
                }
                node = (byte & (1 << bit)) ? node->right : node->left;
                if (!node) {
                    std::cerr << "Corrupted Stream: Malformed tree traversal" << std::endl;
                    out_file.close();
                    return false;
                }
                if (!node->left && !node->right) {
                    out_buffer[out_buf_pos++] = static_cast<char>(node->ch);
                    if (out_buf_pos >= BUFFER_SIZE) {
                        if (!flush_out_buffer()) {
                            out_file.close();
                            return false;
                        }
                    }
                    ++written;
                    node = root;
                }
            }
        }
    }

    if (!flush_out_buffer()) {
        out_file.close();
        return false;
    }

    out_file.close();
    in_file.close();

    std::cout << "[US-08] Decompression complete: " << archive_path 
              << " -> " << output_path 
              << " (" << written << " bytes restored)" << std::endl;

    return true;
}
