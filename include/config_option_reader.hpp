#ifndef CONFIG_OPTION_READER_HPP
#define CONFIG_OPTION_READER_HPP

#include "config.hpp"

class ConfigOptionReader
{
private:
    Config *m_config;

public:
    ConfigOptionReader(Config *config);
    bool read(int argc, const char **argv);
};

#endif
