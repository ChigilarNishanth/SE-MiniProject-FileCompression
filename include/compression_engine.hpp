#ifndef COMPRESSION_ENGINE_HPP
#define COMPRESSION_ENGINE_HPP

#include "common.hpp"
#include <string>

class CompressionEngine {
public:
    static bool compress(const std::string& input_path, const std::string& output_path);
    static bool decompress(const std::string& archive_path, const std::string& output_path);
};

#endif // COMPRESSION_ENGINE_HPP