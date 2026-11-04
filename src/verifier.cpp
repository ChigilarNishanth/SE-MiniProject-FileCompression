#include "verifier.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

bool LosslessVerifier::compare_files(const std::string& file1_path, const std::string& file2_path, uint64_t& mismatch_offset) {
    std::ifstream f1(file1_path, std::ios::binary | std::ios::ate);
    std::ifstream f2(file2_path, std::ios::binary | std::ios::ate);

    if (!f1.is_open() || !f2.is_open()) {
        std::cerr << "[US-10 Error] Unable to open comparison file: " 
                  << (!f1.is_open() ? file1_path : file2_path) << std::endl;
        return false;
    }

    std::streamsize size1 = f1.tellg();
    std::streamsize size2 = f2.tellg();

    if (size1 != size2) {
        std::cerr << "[US-10 Mismatch] File sizes do not match! Original: " 
                  << size1 << " bytes, Restored: " << size2 << " bytes." << std::endl;
        return false;
    }

    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);

    constexpr size_t BUFFER_SIZE = 4096;
    std::vector<char> buf1(BUFFER_SIZE);
    std::vector<char> buf2(BUFFER_SIZE);

    uint64_t current_offset = 0;

    while (f1 && f2) {
        f1.read(buf1.data(), BUFFER_SIZE);
        f2.read(buf2.data(), BUFFER_SIZE);

        std::streamsize bytes1 = f1.gcount();
        std::streamsize bytes2 = f2.gcount();

        if (bytes1 != bytes2) {
            mismatch_offset = current_offset + static_cast<uint64_t>(std::min(bytes1, bytes2));
            return false;
        }

        for (std::streamsize i = 0; i < bytes1; ++i) {
            if (buf1[i] != buf2[i]) {
                mismatch_offset = current_offset + static_cast<uint64_t>(i);
                return false;
            }
        }

        current_offset += static_cast<uint64_t>(bytes1);
    }

    return true;
}