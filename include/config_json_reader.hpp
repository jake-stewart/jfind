#ifndef CONFIG_JSON_READER_HPP
#define CONFIG_JSON_READER_HPP

#include "config.hpp"
#include "json_reader.hpp"

#include <fstream>

class ConfigJsonReader {
    public:
        bool read(std::ifstream &ifs);
        const std::string& getError() const;
        int getErrorLine() const;
        ConfigJsonReader(StyleManager *styleManager);

    private:
        std::string m_error;
        int m_errorLine;
        StyleManager *m_styleManager;
        Config& m_config = Config::instance();

        std::map<std::string, JsonReaderStrategy*> createOptions();
        bool readJsonFile(JsonElement **root, std::ifstream& ifs);
};


#endif
