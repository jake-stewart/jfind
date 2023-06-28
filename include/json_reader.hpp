#ifndef JSON_READER_HPP
#define JSON_READER_HPP

#include "item_matcher.hpp"
#include "json_parser.hpp"
#include "style_manager.hpp"
#include "choice.hpp"
#include <optional>
#include <map>
#include <vector>

struct JsonError
{
    int line;
    std::string message;

    void typeError(int line, std::string name, std::string type);
};

class JsonReaderStrategy
{
public:
    virtual bool read(
        const std::string &name, JsonElement *element, JsonError *error
    ) = 0;
    virtual ~JsonReaderStrategy(){};
};

class JsonStringReaderStrategy : public JsonReaderStrategy
{
public:
    JsonStringReaderStrategy(std::string *value);
    JsonStringReaderStrategy *min(int minLen);
    JsonStringReaderStrategy *max(int maxLen);
    bool read(const std::string &name, JsonElement *element, JsonError *error)
        override;

private:
    std::string *m_value;
    std::optional<int> m_minLen;
    std::optional<int> m_maxLen;
};

class JsonIntReaderStrategy : public JsonReaderStrategy
{
public:
    JsonIntReaderStrategy(int *value);
    JsonIntReaderStrategy *min(int min);
    JsonIntReaderStrategy *max(int max);
    bool read(const std::string &name, JsonElement *element, JsonError *error)
        override;

private:
    int *m_value;
    std::optional<int> m_min;
    std::optional<int> m_max;
};

class JsonBoolReaderStrategy : public JsonReaderStrategy
{
public:
    JsonBoolReaderStrategy(bool *value);
    bool read(const std::string &name, JsonElement *element, JsonError *error)
        override;

private:
    bool *m_value;
};

struct JsonStyleContext {
    int *style;
    bool blend = false;
};

class JsonStylesReaderStrategy : public JsonReaderStrategy
{
public:
    JsonStylesReaderStrategy(std::map<std::string, JsonStyleContext> &styles);
    bool read(const std::string &name, JsonElement *element, JsonError *error)
        override;
    bool readStyle(JsonElement *element, AnsiStyle *style);
    bool readColorRGB(ColorRGB *color, JsonString *str);
    bool readColor16(Color16 *color, JsonString *str);
    bool readColor(JsonElement *elem, AnsiStyle *style, bool fg);
    bool readAttr(JsonArray *arr, AnsiStyle *style);

private:
    std::map<std::string, JsonStyleContext> m_styles;
    JsonError *m_error;
};

template <typename T>
class JsonEnumReaderStrategy : public JsonReaderStrategy
{
public:
    JsonEnumReaderStrategy(T *value, std::vector<Choice<T>> choices) {
        m_choices = choices;
        m_value = value;
    }

    bool read(const std::string &name, JsonElement *element, JsonError *error)
        override {
        if (element->getType() != STRING) {
            error->typeError(element->getLine(), name, "string");
            return false;
        }
        std::string key = ((JsonString *)element)->getValue();

        int idx = -1;
        for (int i = 0; i < m_choices.size(); i++) {
            if (m_choices[i].name == key) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            std::string message = name + " expects either ";
            int i = 0;
            for (const Choice<T> &choice : m_choices) {
                if (i == m_choices.size() - 1) {
                    message += " or ";
                }
                else if (i > 0) {
                    message += ", ";
                }
                message += "'" + choice.name + "'";
                i++;
            }
            error->message = message;
            error->line = element->getLine();
            return false;
        }
        *m_value = m_choices[idx].value;
        return true;
    }

private:
    T *m_value;
    std::vector<Choice<T>> m_choices;
};

class JsonStringArrayReaderStrategy : public JsonReaderStrategy
{
public:
    JsonStringArrayReaderStrategy(std::vector<std::string> *value);
    JsonStringArrayReaderStrategy *min(int min);
    JsonStringArrayReaderStrategy *max(int max);
    bool read(const std::string &name, JsonElement *element, JsonError *error)
        override;

private:
    std::vector<std::string> *m_value;
    std::optional<int> m_min;
    std::optional<int> m_max;
};


class JsonReader
{
public:
    JsonReader(std::map<std::string, JsonReaderStrategy *> &options);
    bool read(JsonElement *element);
    const JsonError &getError() const;

private:
    JsonError m_error;
    std::map<std::string, JsonReaderStrategy *> m_options;
};

#endif
