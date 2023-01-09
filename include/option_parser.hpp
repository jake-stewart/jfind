#ifndef OPTION_PARSER_HPP
#define OPTION_PARSER_HPP

#include <string>
#include <vector>
#include <optional>
#include "option.hpp"

struct KeywordArg {
    std::optional<std::string> key;
    std::optional<std::string> value;
};

class OptionParser {
    public:
        OptionParser(std::vector<Option*>& options);
        void expectPositionalArgs(int min, int max);
        std::vector<std::string>& getPositionalArgs();
        bool parse(int argc, const char **argv);

    private:
        std::vector<Option*> options;
        std::vector<std::string> positionalArgs;
        int minPositionalArgs;
        int maxPositionalArgs;
};

#endif
