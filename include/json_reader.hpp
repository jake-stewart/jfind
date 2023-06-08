#ifndef JSON_READER_HPP
#define JSON_READER_HPP

#include "json_parser.hpp"
#include "style_manager.hpp"
#include <optional>

class JsonReaderStrategy
{
    public:
        virtual bool read(const std::string& name, JsonElement *element) = 0;
};


class JsonStringReaderStrategy : public JsonReaderStrategy {
    public:
        JsonStringReaderStrategy(std::string *value);
        JsonStringReaderStrategy* min(int minLen);
        JsonStringReaderStrategy* max(int maxLen);
        bool read(const std::string& name, JsonElement *element) override;

    private:
        std::string* m_value;
        std::optional<int> m_minLen;
        std::optional<int> m_maxLen;
};

class JsonIntReaderStrategy : public JsonReaderStrategy {
    public:
        JsonIntReaderStrategy(int *value);
        JsonIntReaderStrategy* min(int min);
        JsonIntReaderStrategy* max(int max);
        bool read(const std::string& name, JsonElement *element) override;

    private:
        int* m_value;
        std::optional<int> m_min;
        std::optional<int> m_max;
};

class JsonBoolReaderStrategy : public JsonReaderStrategy {
    public:
        JsonBoolReaderStrategy(bool *value);
        bool read(const std::string& name, JsonElement *element) override;

    private:
        bool* m_value;
};

class JsonStylesReaderStrategy : public JsonReaderStrategy {
    public:
        JsonStylesReaderStrategy(StyleManager *styleManager,
                std::map<std::string, int*>& styles);
        bool read(const std::string& name, JsonElement *element) override;
        bool readStyle(JsonElement *element, AnsiStyle *style);
        bool readColorRGB(ColorRGB *color, JsonString *str);
        bool readColor16(Color16 *color, JsonString *str);
        bool readColor(JsonElement *elem, AnsiStyle *style, bool fg);
        bool readAttr(JsonArray *arr, AnsiStyle *style);

    private:
        StyleManager *m_styleManager;
        std::map<std::string, int*> m_styles;
};

class JsonReader {
    public:
        JsonReader(std::map<std::string, JsonReaderStrategy*>& options);
        bool read(JsonElement *element);
        static const std::string& getError();
        static int getErrorLine();

    private:
        std::map<std::string, JsonReaderStrategy*> m_options;
};

#endif
