#ifndef ARCHIVE_IO_HPP
#define ARCHIVE_IO_HPP

#include "common.hpp"
#include <fstream>

class ArchiveIO {
public:
    static bool write_header(std::ofstream& out_stream, uint64_t original_size, const FrequencyTable& freq_table);
    static bool read_header(std::ifstream& in_stream, uint64_t& out_original_size, FrequencyTable& out_freq_table);
};

#endif // ARCHIVE_IO_HPP