#include "bit_stream.hpp"

void BitWriter::write_bit(uint8_t bit) {
    buffer = (buffer << 1) | (bit & 1);
    bit_count++;
    if (bit_count == 8) {
        out.put(static_cast<char>(buffer));
        buffer = 0;
        bit_count = 0;
    }
}

void BitWriter::write_code(const std::string& code) {
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