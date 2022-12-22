#ifndef CONFIG_JSON_READER_HPP
#define CONFIG_JSON_READER_HPP

#include "config.hpp"
#include "json_reader.hpp"

class ConfigJsonReader
{
private:
    std::string  m_error;
    int          m_error_line;
    StyleManager *m_style_manager;
    Config       *m_config;

    std::map<std::string, JsonReaderStrategy*> createOptions();
    bool readJsonFile(JsonElement **root, const char *file);

public:
    ConfigJsonReader(Config *config, StyleManager *style_manager);
    bool read(const char *file);
    std::string& getError();
    int getErrorLine();
};


#endif
