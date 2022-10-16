#include "../include/option.hpp"

std::vector<Option> parseOptions(int argc, const char **argv) {
    std::vector<Option> options;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        Option option;

        if (arg.starts_with("--")) {
            arg = arg.substr(2);

            int it = arg.find("=");
            if (it != std::string::npos) {
                option.key = arg.substr(0, it);
                option.value = arg.substr(it + 1, arg.size() - it - 1);
                option.hasValue = true;
            }
            else {
                option.key = arg;
            }
        }
        else {
            option.value = arg;
            option.hasValue = true;
        }

        options.push_back(option);
    }

    return options;
}
