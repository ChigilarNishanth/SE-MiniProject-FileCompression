#include "bit_stream.hpp"
#include <filesystem>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
void expect(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

std::string read_bytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    expect(in.is_open(), "could not read bitstream fixture");
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void check_boundary(const std::string& path, const std::string& bits,
                    const std::string& expected) {
    {
        std::ofstream out(path, std::ios::binary);
        expect(out.is_open(), "could not write bitstream fixture");
        BitWriter writer(out);
        writer.write_code(bits);
        writer.flush();
    }
    expect(read_bytes(path) == expected, "unexpected bytes at bit boundary");
    std::cout << "PASS: " << bits.size() << " input bits produce "
              << expected.size() << " packed bytes" << std::endl;
}
}

int main(int argc, char** argv) {
    if (argc != 2) return 2;
    const std::string path = argv[1];
    try {
        check_boundary(path, "", "");
        check_boundary(path, "1", "\x80");
        check_boundary(path, "1111111", "\xfe");
        check_boundary(path, "10110010", "\xb2");
        check_boundary(path, "101100101", "\xb2\x80");
        {
            std::ofstream out(path, std::ios::binary);
            expect(out.is_open(), "could not write bitstream fixture");
            BitWriter writer(out);
            writer.write_code("10110010");
            writer.write_code("111");
            writer.flush();
            writer.flush();
            writer.write_code("1");

            bool rejected = false;
            try { writer.write_code("10x"); }
            catch (const std::invalid_argument&) { rejected = true; }
            expect(rejected, "invalid code was accepted");

            rejected = false;
            try { writer.write_bit(2); }
            catch (const std::invalid_argument&) { rejected = true; }
            expect(rejected, "invalid bit was accepted");
        }
        expect(read_bytes(path) == std::string("\xb2\xe0\x80", 3),
               "flush or invalid input changed the packed bytes");
        std::cout << "PASS: repeated flush, destructor flush and atomic rejection of invalid inputs"
                  << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << std::endl;
        return 1;
    }
    std::filesystem::remove(path);
    return 0;
}
