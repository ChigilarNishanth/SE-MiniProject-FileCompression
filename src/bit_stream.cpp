#include "bit_stream.hpp"
#include <stdexcept>

void BitWriter::write_bit(uint8_t bit) {
    if (bit > 1) throw std::invalid_argument("bit must be 0 or 1");
    buffer = static_cast<uint8_t>((buffer << 1) | bit);
    bit_count++;
    if (bit_count == 8) {
        out.put(static_cast<char>(buffer));
        buffer = 0;
        bit_count = 0;
    }
}

void BitWriter::write_code(const std::string& code) {
    if (code.find_first_not_of("01") != std::string::npos)
        throw std::invalid_argument("code must contain only 0 and 1");
    for (char c : code) {
        write_bit(c == '1' ? 1 : 0);
    }
}

void BitWriter::flush() {
    if (bit_count > 0) {
        buffer <<= (8 - bit_count);
        out.put(static_cast<char>(buffer));
        buffer = 0;
        bit_count = 0;
    }
}