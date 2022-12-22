#include <fstream>
#include "../include/option.hpp"
#include "../include/config_option_reader.hpp"

ConfigOptionReader::ConfigOptionReader(Config *config) {
    m_config = config;
}

#define BOOLEAN_ARG(ARG, NAME) \
    else if (option.key == ARG) { \
        if (option.hasValue) { \
            printf("Option '%s' did not expect a value\n", option.key.c_str()); \
            return false; \
        } \
        m_config->NAME = true; \
    }

#define STRING_ARG(ARG, NAME) \
    else if (option.key == ARG) { \
        if (!option.hasValue) { \
            printf("Option '%s' expected a value\n", option.key.c_str()); \
            return false; \
        } \
        m_config->NAME = option.value; \
    } \

bool ConfigOptionReader::read(int argc, const char **argv) {
    std::vector<Option> options = parseOptions(argc, argv);
    for (int i = 0; i < options.size(); i++) {
        Option& option = options[i];

        if (option.key.empty()) {
            if (!m_config->sourceFile.empty()) {
                printf("Invalid argument at position %d: '%s'\n", i + 1, option.value.c_str());
                return false;
            }
            m_config->sourceFile = option.value;
        }
        BOOLEAN_ARG("hints", showHints)
        BOOLEAN_ARG("select-hint", selectHint)
        BOOLEAN_ARG("select-both", selectBoth)
        BOOLEAN_ARG("accept-non-match", acceptNonMatch)
        STRING_ARG("history", historyFile)
        STRING_ARG("prompt", prompt)
        STRING_ARG("query", query)
        else {
            printf("Unknown option: '--%s'\n", option.key.c_str());
            return false;
        }
    }
    return true;
}
