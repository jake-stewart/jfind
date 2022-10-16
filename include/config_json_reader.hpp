#ifndef CONFIG_JSON_READER_HPP
#define CONFIG_JSON_READER_HPP

#include "config.hpp"
#include "json_parser.hpp"

class ConfigJsonReader
{
    public:
        bool read(const char *file, Config *config);
        std::string getError();
        int getErrorLine();

    private:
        const char *file;
        int errorLine = 0;
        std::string error;
        Config *config;

        bool readRoot(JsonObject *obj);

        bool readStyles(JsonObject *obj);
        bool readStyle(JsonObject *obj, AnsiStyle *style);
        Color16 readColor(JsonString *str);
        bool readAttr(JsonArray *arr, AnsiStyle *style);
};

#endif
