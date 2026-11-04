#ifndef DECOMPRESSION_ENGINE_HPP
#define DECOMPRESSION_ENGINE_HPP

#include "common.hpp"
#include <string>

// US-08 / FR-02: Decompression Engine & Safe Bounds Tree Traversal
class DecompressionEngine {
public:
    /**
     * @brief Decompresses a .czip archive to restore the original file losslessly.
     *        Reconstructs the Huffman tree from the header frequency table and
     *        traverses the tree bit-by-bit with strict bounds checking.
     *
     * @param archive_path Path to the input .czip archive.
     * @param output_path Path to write the restored file.
     * @return true on success, false on I/O error, invalid header, or malformed bitstream.
     */
    static bool decompress(const std::string& archive_path, const std::string& output_path);
};

#endif // DECOMPRESSION_ENGINE_HPP
