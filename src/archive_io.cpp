#include "archive_io.hpp"

bool ArchiveIO::write_header(std::ofstream& out_stream, uint64_t original_size, const FrequencyTable& freq_table) {
    if (!out_stream) {
        return false;
    }

    ArchiveHeader header;
    header.magic = CZIP_MAGIC;
    header.original_size = original_size;

    out_stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
    out_stream.write(reinterpret_cast<const char*>(freq_table.counts), sizeof(freq_table.counts));

    return static_cast<bool>(out_stream);
}

bool ArchiveIO::read_header(std::ifstream& in_stream, uint64_t& out_original_size, FrequencyTable& out_freq_table) {
    if (!in_stream) {
        return false;
    }

    ArchiveHeader header;
    in_stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!in_stream) {
        std::cerr << "Corrupted Stream: Unexpected EOF" << std::endl;
        return false;
    }

    if (header.magic != CZIP_MAGIC) {
        std::cerr << "Security Error: Invalid header signature" << std::endl;
        return false;
    }

    in_stream.read(reinterpret_cast<char*>(out_freq_table.counts), sizeof(out_freq_table.counts));
    if (!in_stream) {
        std::cerr << "Corrupted Stream: Unexpected EOF" << std::endl;
        return false;
    }

    out_original_size = header.original_size;
    return true;
}
