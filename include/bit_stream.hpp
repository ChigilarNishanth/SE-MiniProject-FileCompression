#ifndef BIT_STREAM_HPP
#define BIT_STREAM_HPP

#include <fstream>
#include <cstdint>
#include <string>

class BitWriter {
private:
    std::ofstream& out;
    uint8_t buffer = 0;
    uint8_t bit_count = 0;

public:
    explicit BitWriter(std::ofstream& os) : out(os) {}
    ~BitWriter() { flush(); }

    void write_bit(uint8_t bit);
    void write_code(const std::string& code);
    void flush();
};

#endif // BIT_STREAM_HPP