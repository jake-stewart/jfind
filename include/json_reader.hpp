#ifndef JSON_READER_HPP
#define JSON_READER_HPP

#include "json_parser.hpp"
#include "style_manager.hpp"
#include "item_matcher.hpp"
#include <optional>

struct JsonError {
    int line;
    std::string message;

    void typeError(int line, std::string name, std::string type);
};

class JsonReaderStrategy
{
    public:
        virtual bool read(const std::string& name, JsonElement *element, JsonError *error) = 0;
        virtual ~JsonReaderStrategy() {};

    private:
};


class JsonStringReaderStrategy : public JsonReaderStrategy {
    public:
        JsonStringReaderStrategy(std::string *value);
        JsonStringReaderStrategy* min(int minLen);
        JsonStringReaderStrategy* max(int maxLen);
        bool read(const std::string& name, JsonElement *element, JsonError *error) override;

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
        bool read(const std::string& name, JsonElement *element, JsonError *error) override;

    private:
        int* m_value;
        std::optional<int> m_min;
        std::optional<int> m_max;
};

class JsonBoolReaderStrategy : public JsonReaderStrategy {
    public:
        JsonBoolReaderStrategy(bool *value);
        bool read(const std::string& name, JsonElement *element, JsonError *error) override;

    private:
        bool* m_value;
};

class JsonStylesReaderStrategy : public JsonReaderStrategy {
    public:
        JsonStylesReaderStrategy(StyleManager *styleManager,
                std::map<std::string, int*>& styles);
        bool read(const std::string& name, JsonElement *element, JsonError *error) override;
        bool readStyle(JsonElement *element, AnsiStyle *style);
        bool readColorRGB(ColorRGB *color, JsonString *str);
        bool readColor16(Color16 *color, JsonString *str);
        bool readColor(JsonElement *elem, AnsiStyle *style, bool fg);
        bool readAttr(JsonArray *arr, AnsiStyle *style);

    private:
        StyleManager *m_styleManager;
        std::map<std::string, int*> m_styles;
        JsonError *m_error;
};


template <typename T>
class JsonEnumReaderStrategy : public JsonReaderStrategy {
    public:
        JsonEnumReaderStrategy(T *value, std::map<std::string, T> lookup) {
            m_lookup = lookup;
            m_value = value;
        }

        bool read(const std::string& name, JsonElement *element, JsonError *error) override {
            if (element->getType() != STRING) {
                error->typeError(element->getLine(), name, "string");
                return false;
            }
            std::string key = ((JsonString*)element)->getValue();

            auto it = m_lookup.find(key);

            if (it == m_lookup.end()) {
                std::string message = name + " expects either ";
                int i = 0;
                for (auto it : m_lookup) {
                    if (i == m_lookup.size() - 1) {
                        message += " or ";
                    }
                    else if (i > 0) {
                        message += ", ";
                    }
                    message += "'" + it.first + "'";
                    i++;
                }
                error->message = message;
                error->line = element->getLine();
                return false;
            }
            *m_value = it->second;
            return true;
        }

    private:
        T *m_value;
        std::map<std::string, T> m_lookup;
};

class JsonReader {
    public:
        JsonReader(std::map<std::string, JsonReaderStrategy*>& options);
        bool read(JsonElement *element);
        const JsonError& getError() const;

    private:
        JsonError m_error;
        std::map<std::string, JsonReaderStrategy*> m_options;
};

#endif
