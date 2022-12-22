#ifndef JSON_READER_HPP
#define JSON_READER_HPP

#include "json_parser.hpp"
#include "style_manager.hpp"

class JsonReaderStrategy
{
public:
    virtual bool read(const std::string& name, JsonElement *element) = 0;
};


class JsonStringReaderStrategy : public JsonReaderStrategy
{
private:
    std::string* m_value;
    std::optional<int> m_min_len;
    std::optional<int> m_max_len;

public:
    JsonStringReaderStrategy(std::string *value);
    JsonStringReaderStrategy* min(int length);
    JsonStringReaderStrategy* max(int length);
    bool read(const std::string& name, JsonElement *element);
};

class JsonIntReaderStrategy : public JsonReaderStrategy
{
private:
    int* m_value;
    std::optional<int> m_min_len;
    std::optional<int> m_max_len;

public:
    JsonIntReaderStrategy(int *value);
    JsonIntReaderStrategy* min(int length);
    JsonIntReaderStrategy* max(int length);
    bool read(const std::string& name, JsonElement *element);
};

class JsonStylesReaderStrategy : public JsonReaderStrategy
{
private:
    StyleManager *m_style_manager;
    std::map<std::string, int*> m_styles;

public:
    JsonStylesReaderStrategy(StyleManager *style_manager, std::map<std::string, int*>& styles);
    bool read(const std::string& name, JsonElement *element);
    bool readStyle(JsonElement *element, AnsiStyle *style);
    bool readColorRGB(ColorRGB *color, JsonString *str);
    bool readColor16(Color16 *color, JsonString *str);
    bool readColor(JsonString *str, AnsiStyle *style, bool fg);
    bool readAttr(JsonArray *arr, AnsiStyle *style);
};

class JsonReader
{
private:
    std::map<std::string, JsonReaderStrategy*> m_options;

public:
    JsonReader(std::map<std::string, JsonReaderStrategy*>& options);
    bool read(JsonElement *element);

    static std::string& getError();
    static int getErrorLine();
};

#endif
