#ifndef CONFIG_OPTION_READER_HPP
#define CONFIG_OPTION_READER_HPP

#include "config.hpp"

class ConfigOptionReader {
    public:
        ConfigOptionReader(Config *config);
        bool read(int argc, const char **argv);

    private:
        Config *m_config;
};

#endif
