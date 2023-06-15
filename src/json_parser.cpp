#include "../include/json_parser.hpp"
#include <regex>
#include <fstream>

JsonParser::~JsonParser() {
    for (const JsonElement *element : m_elements) {
        delete element;
    }
}

JsonElementType JsonElement::getType() const {
    return m_type;
}

int JsonElement::getLine() const {
    return m_line;
}

JsonString::JsonString(int line, int idx, std::string value) {
    m_line = line;
    m_idx = idx;
    m_value = value;
    m_type = STRING;
}

std::string& JsonString::getValue() {
    return m_value;
}

std::string JsonString::repr(int indent, int depth) {
    std::string escaped = m_value;
    escaped = std::regex_replace(escaped, std::regex("\\\\"), "\\\\");
    escaped = std::regex_replace(escaped, std::regex("\n"), "\\n");
    escaped = std::regex_replace(escaped, std::regex("\""), "\\\"");
    escaped = std::regex_replace(escaped, std::regex("\t"), "\\t");
    return '"' + escaped + '"';
}


JsonInt::JsonInt(int line, int idx, int value) {
    m_line = line;
    m_idx = idx;
    m_value = value;
    m_type = INT;
}

int& JsonInt::getValue() {
    return m_value;
}

std::string JsonInt::repr(int indent, int depth) {
    return std::to_string(m_value);
}


JsonFloat::JsonFloat(int line, int idx, double value) {
    m_line = line;
    m_idx = idx;
    m_value = value;
    m_type = FLOAT;
}

double& JsonFloat::getValue() {
    return m_value;
}

std::string JsonFloat::repr(int indent, int depth) {
    return std::to_string(m_value);
}


JsonBoolean::JsonBoolean(int line, int idx, bool value) {
    m_line = line;
    m_idx = idx;
    m_value = value;
    m_type = BOOLEAN;
}

bool& JsonBoolean::getValue() {
    return m_value;
}

std::string JsonBoolean::repr(int indent, int depth) {
    return m_value ? "true" : "false";
}


JsonObject::JsonObject(int line, int idx) {
    m_line = line;
    m_idx = idx;
    m_type = OBJECT;
}

std::map<std::string, JsonObjectEntry>& JsonObject::getValue() {
    return m_value;
}

std::string JsonObject::repr(int indent, int depth) {
    std::string repr = "{";

    for (auto it = m_value.begin(); it != m_value.end(); ++it) {
        if (indent) {
            repr += "\n";
            for (int i = 0; i < indent * (depth + 1); i++) {
                repr += " ";
            }
        }
        repr += "\"" + it->first + "\":";
        if (indent) {
            repr += " ";
        }
        repr += it->second.value->repr(indent, depth + 1);
        if (std::next(it) != m_value.end()) {
            repr += ",";
        }
        if (indent) {
            repr += " ";
        }
    }
    if (indent) {
        repr += "\n";
        for (int i = 0; i < indent * depth; i++) {
            repr += " ";
        }
    }

    repr += "}";
    return repr;
}


JsonArray::JsonArray(int line, int idx) {
    m_line = line;
    m_idx = idx;
    m_type = ARRAY;
}

std::vector<JsonElement*>& JsonArray::getValue() {
    return m_value;
}

std::string JsonArray::repr(int indent, int depth) {
    std::string repr = "[";

    std::vector<JsonElement*>::iterator it;
    for (it = m_value.begin(); it != m_value.end(); ++it) {
        if (indent) {
            repr += "\n";
            for (int i = 0; i < indent * (depth + 1); i++) {
                repr += " ";
            }
        }
        repr += (*it)->repr(indent, depth + 1);
        if (std::next(it) != m_value.end()) {
            repr += ",";
            if (indent) {
                repr += " ";
            }
        }
    }

    if (indent) {
        repr += "\n";
        for (int i = 0; i < indent * depth; i++) {
            repr += " ";
        }
    }

    repr += "]";
    return repr;
}


bool JsonParser::parse(std::string json) {
    m_idx = 0;
    m_line = 1;
    m_json = json;

    skipWhitespace();

    if (!peek()) {
        return true;
    }

    bool valid = parseElement();

    if (valid) {
        skipWhitespace();
        if (m_idx != json.length()) {
            m_error = "Expected EOF";
            valid = false;
        }
    }

    return valid;
}

std::string JsonParser::getError() const {
    return m_error;
}

int JsonParser::getLine() const {
    return m_line;
}

JsonElement* JsonParser::getElement() const {
    return m_elements.size() ? m_elements.back() : nullptr;
}

void JsonParser::skipWhitespace() {
    while (m_idx < m_json.length() && (m_json[m_idx] == ' '
                || m_json[m_idx] == '\t'
                || m_json[m_idx] == '\n'
                || m_json[m_idx] == '\r'))
    {
        if (m_json[m_idx] == '\n') {
            m_line++;
        }
        m_idx++;
    }
}

char JsonParser::peek() {
    if (m_idx >= m_json.length()) {
        return 0;
    }
    return m_json[m_idx];
}

char JsonParser::next() {
    if (m_idx >= m_json.length()) {
        return 0;
    }
    if (m_json[m_idx] == '\n') {
        m_line++;
    }
    return m_json[m_idx++];
}

bool JsonParser::parseElement() {
    skipWhitespace();
    switch (peek()) {
        case '{':
            return parseObject();
        case '[':
            return parseArray();
        case '"':
            return parseString();
        case '-':
        case '0' ... '9':
            return parseNumber();
        case 0:
            m_error = "Unexpected EOF";
            return false;
        default:
            return parseKeyword();
    }
}

bool JsonParser::parseObject() {
    JsonObject *object = new JsonObject(m_line, m_idx);
    bool valid = true;

    if (next() != '{') {
        m_error = "An object must start with an open brace";
        valid = false;
    }
    if (valid) {
        skipWhitespace();
        while (peek() != '}') {
            if (!parseString()) {
                valid = false;
                break;
            }
            JsonString *key = (JsonString*)m_elements.back();

            skipWhitespace();
            if (next() != ':') {
                m_error = "Object expected a colon separator";
                valid = false;
                break;
            }
            skipWhitespace();

            if (!parseElement()) {
                valid = false;
                break;
            }

            object->getValue()[key->getValue()] = {key, m_elements.back()};

            skipWhitespace();
            if (peek() == '}') {
                break;
            }
            if (next() != ',') {
                m_error = "Object expected a comma separator";
                valid = false;
                break;
            }
            else {
                skipWhitespace();
                if (peek() == '}') {
                    m_error = "Trailing comma in object";
                    valid = false;
                    break;
                }
            }
            skipWhitespace();
        }
    }

    m_elements.push_back(object);
    if (valid) {
        next();
    }
    return valid;
}

bool JsonParser::parseArray() {
    JsonArray *array = new JsonArray(m_line, m_idx);
    bool valid = true;

    if (next() != '[') {
        m_error = "An array must start with an open bracket";
        valid = false;
    }
    if (valid) {
        skipWhitespace();
        while (peek() != ']') {
            if (!parseElement()) {
                valid = false;
                break;
            }

            array->getValue().push_back(m_elements.back());

            skipWhitespace();
            if (peek() == ']') {
                break;
            }
            if (next() != ',') {
                m_error = "Array expected a comma separator";
                valid = false;
                break;
            }
            else {
                skipWhitespace();
                if (peek() == ']') {
                    m_error = "Trailing comma in array";
                    valid = false;
                    break;
                }
            }
            skipWhitespace();
        }
    }
    m_elements.push_back(array);
    if (valid) {
        next();
    }
    return valid;
}

bool JsonParser::parseNumber() {
    bool hasDecimal = false;
    bool parsed = false;
    std::string value;

    if (peek() == '-') {
        value += next();
    }

    while (!parsed) {
        switch (peek()) {
            case '0' ... '9':
                value += next();
                break;
            case '.':
                if (hasDecimal) {
                    m_error = "A number cannot contain multiple decimal points";
                    return false;
                }
                hasDecimal = true;
                value += next();
                break;
            default:
                parsed = true;
                break;
        }
    }

    if (hasDecimal) {
        if (value.front() == '.' || value.back() == '.' || value.length() < 3)
        {
            m_error = "A number cannot start or end with a decimal point";
            return false;
        }
        m_elements.push_back(new JsonFloat(m_line, m_idx, std::stod(value)));
    }
    else {
        if (!value.length() || value == "-") {
            m_error = "Empty number";
            return false;
        }
        try {
            m_elements.push_back(new JsonInt(m_line, m_idx, std::stoi(value)));
        }
        catch (const std::out_of_range&) {
            m_error = "The number is out of range";
            return false;
        }
    }

    return true;
}

bool JsonParser::parseString() {
    std::string value;
    bool escaped = false;

    if (next() != '"') {
        m_error = "A string must start with a double quote";
        return false;
    }
    while (true) {
        if (peek() == 0) {
            m_error = "Unexpected EOF while parsing string";
            return false;
        }
        if (escaped) {
            escaped = false;
            switch (char c = next()) {
                case 'n':
                    value += '\n';
                    break;
                case 't':
                    value += '\t';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '\"':
                    value += '"';
                    break;
                default:
                    value += '\\' + c;
            }
        }
        else if (peek() == '\\') {
            next();
            escaped = true;
        }
        else if (peek() == '"') {
            next();
            break;
        }
        else {
            value += next();
        }
    }

    m_elements.push_back(new JsonString(m_line, m_idx, value));

    return true;
}

bool JsonParser::parseKeyword() {
    std::string value;
    bool parsed = false;
    while (!parsed) {
        switch(peek()) {
            case 'A' ... 'Z':
            case 'a' ... 'z':
                value += next();
                break;
            default:
                parsed = true;
                break;
        }
    }

    if (value == "true") {
        m_elements.push_back(new JsonBoolean(m_line, m_idx, true));
    }
    else if (value == "false") {
        m_elements.push_back(new JsonBoolean(m_line, m_idx, false));
    }
    else {
        m_error = "Unknown keyword: " + value;
        return false;
    }

    return true;
}
