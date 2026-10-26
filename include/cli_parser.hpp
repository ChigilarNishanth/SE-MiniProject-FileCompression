#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include "common.hpp"

class CLIParser {
public:
    static CLIConfig parse(int argc, char* argv[]);
    static void print_usage(const char* program_name);
};

#endif // CLI_PARSER_HPP