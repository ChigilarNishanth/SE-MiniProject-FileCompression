#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <string>
#include <iostream>

// Magic Byte Signature for .czip archives (0x435A -> "CZ")
constexpr uint16_t CZIP_MAGIC = 0x435A;

// Exit Codes
constexpr int EXIT_SUCCESS_CODE = 0;
constexpr int EXIT_FAILURE_CODE = 1; // Execution / I/O / Security Error
constexpr int EXIT_SYNTAX_ERROR  = 2; // Invalid CLI arguments

// Security Error Codes
constexpr int SEC_ERR_BAD_HEADER       = 101;
constexpr int SEC_ERR_MALFORMED_STREAM = 102;

// Command Types
enum class CommandType {
    COMPRESS,
    DECOMPRESS,
    HELP,
    INVALID
};

// Configuration parsed from Command Line Interface
struct CLIConfig {
    CommandType mode = CommandType::INVALID;
    std::string input_path;
    std::string output_path;
};

// Frequency map container (256 byte values: 0x00 to 0xFF)
struct FrequencyTable {
    uint64_t counts[256] = {0};
};

// Archive Header Structure (Serialized to disk)
#pragma pack(push, 1)
struct ArchiveHeader {
    uint16_t magic = CZIP_MAGIC; // Must match 0x435A
    uint64_t original_size = 0;  // Original file size in bytes
};
#pragma pack(pop)

#endif // COMMON_HPP