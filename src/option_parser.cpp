#include "../include/option_parser.hpp"

std::vector<KeywordArg> parseKwargs(int argc, const char **argv) {
    std::vector<KeywordArg> kwargs;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        KeywordArg kwarg;

        if (arg.starts_with("--")) {
            arg = arg.substr(2);

            int it = arg.find("=");
            if (it != std::string::npos) {
                kwarg.key = arg.substr(0, it);
                kwarg.value = arg.substr(it + 1, arg.size() - it - 1);
            }
            else {
                kwarg.key = arg;
            }
        }
        else {
            kwarg.value = arg;
        }

        kwargs.push_back(kwarg);
    }

    return kwargs;
}


OptionParser::OptionParser(std::vector<Option*>& options) {
    this->options = options;
    minPositionalArgs = 0;
    maxPositionalArgs = 0;
}

void OptionParser::expectPositionalArgs(int min, int max) {
    minPositionalArgs = min;
    maxPositionalArgs = max;
}

std::vector<std::string>& OptionParser::getPositionalArgs() {
    return positionalArgs;
}

bool OptionParser::parse(int argc, const char **argv) {
    std::vector<KeywordArg> kwargs = parseKwargs(argc, argv);

    for (KeywordArg& kwarg : kwargs) {
        if (!kwarg.key.has_value()) {
            positionalArgs.push_back(kwarg.value.value_or(""));
            continue;
        }

        Option *matchedOption = nullptr;
        for (Option* option : options) {
            if (option->getKey() != kwarg.key) {
                continue;
            }
            matchedOption = option;
        }
        if (matchedOption == nullptr) {
            fprintf(stderr, "'--%s' is not a recognized option\n",
                    kwarg.key->c_str());
            return false;
        }
        bool success = matchedOption->parse(kwarg.value.has_value()
                ? kwarg.value->c_str() : nullptr);
        if (!success) {
            return false;
        }
    }

    if (minPositionalArgs == maxPositionalArgs) {
        if (positionalArgs.size() != minPositionalArgs) {
            fprintf(stderr,
                    "Expected %d positional %s, but %zu %s provided\n",
                    minPositionalArgs, minPositionalArgs == 1 ? "arg" : "args",
                    positionalArgs.size(),
                    positionalArgs.size() == 1 ? "was" : "were");
            return false;
        }
    }
    else if (positionalArgs.size() < minPositionalArgs) {
        fprintf(stderr,
                "Expected at least %d positional %s, but %zu %s provided\n",
                minPositionalArgs, minPositionalArgs == 1 ? "arg" : "args",
                positionalArgs.size(), positionalArgs.size() == 1
                ? "was" : "were");
        return false;
    }
    else if (maxPositionalArgs != -1
            && positionalArgs.size() > maxPositionalArgs)
    {
        fprintf(stderr,
                "Expected at most %d positional %s, but %zu %s provided\n",
                maxPositionalArgs, maxPositionalArgs == 1 ? "arg" : "args",
                positionalArgs.size(), positionalArgs.size() == 1
                ? "was" : "were");
        return false;
    }

    return true;
}
