#ifndef FREQ_ANALYZER_HPP
#define FREQ_ANALYZER_HPP

#include "common.hpp"
#include <cstdint>
#include <istream>
#include <string>

// US-02 / FR-03: File Stream I/O & 256-Byte Frequency Analyzer
// Reads input files in binary mode (rb) byte-by-byte and calculates a 256-element frequency distribution array.
class FrequencyAnalyzer {
public:
    /**
     * @brief Reads from a binary input stream byte-by-byte and populates
     *        the 256-element frequency distribution array (0x00 to 0xFF).
     *
     * @param in_stream Open input stream in binary mode.
     * @param out_table FrequencyTable struct where counts will be recorded.
     * @param out_total_bytes Output parameter for total bytes read.
     * @return true on success, false on read error.
     */
    static bool analyze_stream(std::istream& in_stream,
                               FrequencyTable& out_table,
                               uint64_t& out_total_bytes);

    /**
     * @brief Opens a file in binary mode (rb), reads byte-by-byte, and populates
     *        the 256-element frequency distribution array (0x00 to 0xFF).
     *
     * @param input_path Path to the binary file to analyze.
     * @param out_table FrequencyTable struct where counts for bytes 0x00-0xFF will be recorded.
     * @param out_total_bytes Output parameter to receive the total number of bytes read.
     * @return true on success, false if the file cannot be opened or read (fails gracefully).
     */
    static bool analyze_file(const std::string& input_path,
                             FrequencyTable& out_table,
                             uint64_t& out_total_bytes);

    /**
     * @brief Overload that analyzes the file without returning total byte count explicitly.
     *
     * @param input_path Path to the binary file to analyze.
     * @param out_table FrequencyTable struct where counts for bytes 0x00-0xFF will be recorded.
     * @return true on success, false if the file cannot be opened or read (fails gracefully).
     */
    static bool analyze_file(const std::string& input_path,
                             FrequencyTable& out_table);
};

#endif // FREQ_ANALYZER_HPP
