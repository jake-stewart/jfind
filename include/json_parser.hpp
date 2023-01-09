#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <string>
#include <map>
#include <vector>

enum JsonElementType {
    OBJECT,
    ARRAY,
    STRING,
    INT,
    FLOAT,
    BOOLEAN
};

class JsonElement {
    public:
        virtual std::string repr(int indent, int depth) = 0;
        JsonElementType getType();
        int getLine();

    protected:
        JsonElementType m_type;
        int m_line;
        int m_idx;
};

class JsonString : public JsonElement {
    public:
        JsonString(int line, int idx, std::string value);
        std::string& getValue();
        std::string repr(int indent, int depth);

    private:
        std::string m_value;
};

class JsonInt : public JsonElement {
    public:
        JsonInt(int line, int idx, int value);
        int& getValue();
        std::string repr(int indent, int depth);

    private:
        int m_value;
};

class JsonFloat : public JsonElement {
    public:
        JsonFloat(int line, int idx, double value);
        double& getValue();
        std::string repr(int indent, int depth);

    private:
        double m_value;
};

class JsonBoolean : public JsonElement {
    public:
        JsonBoolean(int line, int idx, bool value);
        bool& getValue();
        std::string repr(int indent, int depth);

    private:
        bool m_value;
};

struct JsonObjectEntry {
    JsonString *key;
    JsonElement *value;
};

class JsonObject : public JsonElement {
    public:
        JsonObject(int line, int idx);
        std::map<std::string, JsonObjectEntry>& getValue();
        std::string repr(int indent, int depth);

    private:
        std::map<std::string, JsonObjectEntry> m_value;
};

class JsonArray : public JsonElement {
    public:
        JsonArray(int line, int idx);
        std::vector<JsonElement*>& getValue();
        std::string repr(int indent, int depth);

    private:
        std::vector<JsonElement*> m_value;
};


class JsonParser {
    public:
        bool parse(std::string json);
        std::string getError();
        JsonElement* getElement();
        int getLine();

    private:
        std::string m_error;
        int m_line;
        int m_idx;
        std::string m_json;
        std::vector<JsonElement*> m_elements;

        void skipWhitespace();
        char peek();
        char next();
        bool parseElement();
        bool parseObject();
        bool parseArray();
        bool parseNumber();
        bool parseString();
        bool parseKeyword();
};

#endif
