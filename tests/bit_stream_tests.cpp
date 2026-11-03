#include "bit_stream.hpp"
#include <filesystem>
#include <iostream>
#include <iterator>
#include <stdexcept>
void expect(bool ok) { if (!ok) throw std::runtime_error("bit packing assertion failed"); }
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    const std::string path = argv[1];
    {
        std::ofstream out(path, std::ios::binary);
        BitWriter writer(out);
        writer.write_code("10110010");
        writer.write_code("111");
        writer.flush(); writer.flush();
        writer.write_code("1");
        bool rejected = false;
        try { writer.write_code("10x"); } catch (const std::invalid_argument&) { rejected = true; }
        expect(rejected);
        rejected = false;
        try { writer.write_bit(2); } catch (const std::invalid_argument&) { rejected = true; }
        expect(rejected);
    }
    std::ifstream in(path, std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(in)), {});
    expect(bytes == std::string("\xb2\xe0\x80", 3));
    in.close(); std::filesystem::remove(path);
    std::cout << "PASS: byte boundaries, zero padding, repeated flush, destructor flush and invalid codes\n";
}
