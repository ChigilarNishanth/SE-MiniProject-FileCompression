#include "cli_parser.hpp"

CLIConfig CLIParser::parse(int argc, char* argv[]) {
    CLIConfig config;

    if (argc == 2 && std::string(argv[1]) == "--help") {
        config.mode = CommandType::HELP;
        return config;
    }

    if (argc == 4) {
        const std::string command = argv[1];
        config.input_path = argv[2];
        config.output_path = argv[3];

        if (command == "-c") {
            config.mode = CommandType::COMPRESS;
            return config;
        }

        if (command == "-d") {
            config.mode = CommandType::DECOMPRESS;
            return config;
        }
    }

    config.mode = CommandType::INVALID;
    return config;
}

void CLIParser::print_usage(const char* program_name) {
    std::cerr << "Usage:\n"
              << "  " << program_name << " -c <input_file> <output.czip>\n"
              << "  " << program_name << " -d <input.czip> <output_file>\n"
              << "  " << program_name << " --help\n";
}
