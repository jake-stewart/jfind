#ifndef CONFIG_JSON_READER_HPP
#define CONFIG_JSON_READER_HPP

#include "config.hpp"
#include "json_reader.hpp"

#include <fstream>

class ConfigJsonReader
{
public:
    bool read(std::ifstream &ifs);
    const JsonError getError() const;

private:
    JsonError m_error;
    std::map<std::string, JsonReaderStrategy *> createOptions();
    bool readJsonFile(JsonParser &parser, std::ifstream &ifs);
};

#endif
