#include "../include/json_parser.hpp"
#include <regex>
#include <fstream>


JsonElementType JsonElement::getType() {
    return type;
}

int JsonElement::getLine() {
    return line;
}

JsonString::JsonString(int line, int idx, std::string value) {
    this->line = line;
    this->idx = idx;
    this->value = value;
    type = STRING;
}

std::string& JsonString::getValue() {
    return value;
}

std::string JsonString::repr(int indent, int depth) {
    std::string escaped = value;
    escaped = std::regex_replace(
        escaped, std::regex("\\\\"), "\\\\");
    escaped = std::regex_replace(
        escaped, std::regex("\n"), "\\n");
    escaped = std::regex_replace(
        escaped, std::regex("\""), "\\\"");
    escaped = std::regex_replace(
        escaped, std::regex("\t"), "\\t");

    return '"' + escaped + '"';
}


JsonInt::JsonInt(int line, int idx, long value) {
    this->line = line;
    this->idx = idx;
    this->value = value;
    type = INT;
}

long& JsonInt::getValue() {
    return value;
}

std::string JsonInt::repr(int indent, int depth) {
    return std::to_string(value);
}



JsonFloat::JsonFloat(int line, int idx, double value) {
    this->line = line;
    this->idx = idx;
    this->value = value;
    type = FLOAT;
}

double& JsonFloat::getValue() {
    return value;
}

std::string JsonFloat::repr(int indent, int depth) {
    return std::to_string(value);
}



JsonBoolean::JsonBoolean(int line, int idx, bool value) {
    this->line = line;
    this->idx = idx;
    this->value = value;
    type = BOOLEAN;
}

bool& JsonBoolean::getValue() {
    return value;
}

std::string JsonBoolean::repr(int indent, int depth) {
    return value ? "true" : "false";
}



JsonObject::JsonObject(int line, int idx) {
    this->line = line;
    this->idx = idx;
    type = OBJECT;
}

std::map<std::string, JsonObjectEntry>& JsonObject::getValue() {
    return value;
}

std::string JsonObject::repr(int indent, int depth) {
    std::string repr = "{";

    for (auto it = value.begin(); it != value.end(); ++it) { 
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
        if (std::next(it) != value.end()) {
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
    this->line = line;
    this->idx = idx;
    type = ARRAY;
}

std::vector<JsonElement*>& JsonArray::getValue() {
    return value;
}

std::string JsonArray::repr(int indent, int depth) {
    std::string repr = "[";

    std::vector<JsonElement*>::iterator it;
    for (it = value.begin(); it != value.end(); ++it) { 
        if (indent) {
            repr += "\n";
            for (int i = 0; i < indent * (depth + 1); i++) {
                repr += " ";
            }
        }
        repr += (*it)->repr(indent, depth + 1);
        if (std::next(it) != value.end()) {
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
    idx = 0;
    line = 1;
    this->json = json;
    skipWhitespace();
    if (!peek()) {
        return true;
    }
    bool valid = parseElement();
    if (valid) {
        skipWhitespace();
        if (idx != json.length()) {
            error = "Expected EOF";
            valid = false;
        }
    }
    return valid;
}

std::string JsonParser::getError() {
    return error;
}

int JsonParser::getLine() {
    return line;
}

JsonElement* JsonParser::getElement() {
    return elements.size() ? elements.back() : nullptr;
}

void JsonParser::skipWhitespace() {
    while (idx < json.length() && (
                json[idx] == ' '
                || json[idx] == '\t'
                || json[idx] == '\n'
                || json[idx] == '\r')
    ) {
        if (json[idx] == '\n') {
            line++;
        }
        idx++;
    }
}

char JsonParser::peek() {
    if (idx >= json.length()) {
        return 0;
    }
    return json[idx];
}

char JsonParser::next() {
    if (idx >= json.length()) {
        return 0;
    }
    if (json[idx] == '\n') {
        line++;
    }
    return json[idx++];
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
            error = "Unexpected EOF";
            return false;
        default:
            return parseKeyword();
    }
}

bool JsonParser::parseObject() {
    JsonObject *object = new JsonObject(line, idx);
    bool valid = true;

    if (next() != '{') {
        error = "An object must start with an open brace";
        valid = false;
    }
    if (valid) {
        skipWhitespace();
        while (peek() != '}') {
            if (!parseString()) {
                valid = false;
                break;
            }
            JsonString *key = (JsonString*)elements.back();

            skipWhitespace();
            if (next() != ':') {
                error = "Object expected a colon separator";
                valid = false;
                break;
            }
            skipWhitespace();

            if (!parseElement()) {
                valid = false;
                break;
            }

            object->getValue()[key->getValue()] = {key, elements.back()};

            skipWhitespace();
            if (peek() == '}') {
                break;
            }
            if (next() != ',') {
                error = "Object expected a comma separator";
                valid = false;
                break;
            }
            else {
                skipWhitespace();
                if (peek() == '}') {
                    error = "Trailing comma in object";
                    valid = false;
                    break;
                }
            }
            skipWhitespace();
        }
    }

    elements.push_back(object);
    if (valid) {
        next();
    }
    return valid;
}

bool JsonParser::parseArray() {
    JsonArray *array = new JsonArray(line, idx);
    bool valid = true;

    if (next() != '[') {
        error = "An array must start with an open bracket";
        valid = false;
    }
    if (valid) {
        skipWhitespace();
        while (peek() != ']') {
            if (!parseElement()) {
                valid = false;
                break;
            }

            array->getValue().push_back(elements.back());

            skipWhitespace();
            if (peek() == ']') {
                break;
            }
            if (next() != ',') {
                error = "Array expected a comma separator";
                valid = false;
                break;
            }
            else {
                skipWhitespace();
                if (peek() == ']') {
                    error = "Trailing comma in array";
                    valid = false;
                    break;
                }
            }
            skipWhitespace();
        }
    }
    elements.push_back(array);
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
                    error = "A number cannot contain multiple decimal points";
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
        if (value.front() == '.'
                || value.back() == '.'
                || value.length() < 3
        ) {
            error = "A number cannot start or end with a decimal point";
            return false;
        }
        elements.push_back(new JsonFloat(line, idx, std::stod(value)));
    }
    else {
        if (!value.length()) {
            error = "Empty number";
            return false;
        }
        elements.push_back(new JsonInt(line, idx, std::stol(value)));
    }

    return true;
}

bool JsonParser::parseString() {
    std::string value;
    bool escaped = false;

    if (next() != '"') {
        error = "A string must start with a double quote";
        return false;
    }
    while (true) {
        if (peek() == 0) {
            error = "Unexpected EOF while parsing string";
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

    elements.push_back(new JsonString(line, idx, value));

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
        elements.push_back(new JsonBoolean(line, idx, true));
    }
    else if (value == "false") {
        elements.push_back(new JsonBoolean(line, idx, false));
    }
    else {
        error = "Unknown keyword: " + value;
        return false;
    }

    return true;
}
