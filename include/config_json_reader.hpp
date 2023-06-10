#ifndef CONFIG_JSON_READER_HPP
#define CONFIG_JSON_READER_HPP

#include "config.hpp"
#include "json_reader.hpp"

#include <fstream>

class ConfigJsonReader {
    public:
        bool read(std::ifstream &ifs);
        const JsonError getError() const;
        ConfigJsonReader(StyleManager *styleManager);

    private:
        JsonError m_error;
        StyleManager *m_styleManager;
        Config& m_config = Config::instance();

        std::map<std::string, JsonReaderStrategy*> createOptions();
        bool readJsonFile(JsonElement **root, std::ifstream& ifs);
};


#endif
