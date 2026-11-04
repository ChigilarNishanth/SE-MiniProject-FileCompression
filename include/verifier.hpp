#ifndef VERIFIER_HPP
#define VERIFIER_HPP

#include <string>
#include <cstdint>

class LosslessVerifier {
public:
    // Compares two files byte-by-byte. Returns true if 100% identical.
    static bool compare_files(const std::string& file1_path, const std::string& file2_path, uint64_t& mismatch_offset);
};

#endif // VERIFIER_HPP