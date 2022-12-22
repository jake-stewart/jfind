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
    protected:
        JsonElementType type;
        int line;
        int idx;

    public:
        virtual std::string repr(int indent, int depth) = 0;
        JsonElementType getType();
        int getLine();
};

class JsonString : public JsonElement {
    std::string value;

    public:
        JsonString(int line, int idx, std::string value);
        std::string& getValue();
        std::string repr(int indent, int depth);
};

class JsonInt : public JsonElement {
    long value;

    public:
        JsonInt(int line, int idx, long value);
        long& getValue();
        std::string repr(int indent, int depth);
};

class JsonFloat : public JsonElement {
    double value;

    public:
        JsonFloat(int line, int idx, double value);
        double& getValue();
        std::string repr(int indent, int depth);
};

class JsonBoolean : public JsonElement {
    bool value;

    public:
        JsonBoolean(int line, int idx, bool value);
        bool& getValue();
        std::string repr(int indent, int depth);
};

struct JsonObjectEntry {
    JsonString *key;
    JsonElement *value;
};

class JsonObject : public JsonElement {
    std::map<std::string, JsonObjectEntry> value;

    public:
        JsonObject(int line, int idx);
        std::map<std::string, JsonObjectEntry>& getValue();
        std::string repr(int indent, int depth);
};

class JsonArray : public JsonElement {
    std::vector<JsonElement*> value;

    public:
        JsonArray(int line, int idx);
        std::vector<JsonElement*>& getValue();
        std::string repr(int indent, int depth);
};


class JsonParser {
    public:
        bool parse(std::string json);
        std::string getError();
        JsonElement* getElement();
        int getLine();

    private:
        std::string error;
        int line;
        int idx;
        std::string json;
        std::vector<JsonElement*> elements;

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
