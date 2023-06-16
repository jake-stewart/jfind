#include "../include/json_reader.hpp"
#include "../include/util.hpp"

void JsonError::typeError(int line, std::string name, std::string type) {
    this->line = line;
    message = "The datatype for '" + name + "' must be " +
        (isVowel(type[0]) ? "an" : "a") + " " + type;
}

JsonStringReaderStrategy::JsonStringReaderStrategy(std::string *value) {
    m_value = value;
}

JsonStringReaderStrategy *JsonStringReaderStrategy::min(int minLen) {
    m_minLen = minLen;
    return this;
}

JsonStringReaderStrategy *JsonStringReaderStrategy::max(int maxLen) {
    m_maxLen = maxLen;
    return this;
}

bool JsonStringReaderStrategy::read(
    const std::string &name, JsonElement *element, JsonError *error
) {
    if (element->getType() != STRING) {
        error->typeError(element->getLine(), name, "string");
        return false;
    }
    *m_value = ((JsonString *)element)->getValue();
    if (m_minLen.has_value() && m_value->length() < m_minLen.value()) {
        error->message = "The value for '" + name +
            "' cannot have a length less than " +
            std::to_string(m_minLen.value());
        error->line = element->getLine();
        return false;
    }
    if (m_maxLen.has_value() && m_value->length() > m_maxLen.value()) {
        error->message = "The value for '" + name +
            "' cannot have a length greater than " +
            std::to_string(m_maxLen.value());
        error->line = element->getLine();
        return false;
    }
    return true;
}

JsonIntReaderStrategy::JsonIntReaderStrategy(int *value) {
    m_value = value;
}

JsonIntReaderStrategy *JsonIntReaderStrategy::min(int min) {
    m_min = min;
    return this;
}

JsonIntReaderStrategy *JsonIntReaderStrategy::max(int max) {
    m_max = max;
    return this;
}

bool JsonIntReaderStrategy::read(
    const std::string &name, JsonElement *element, JsonError *error
) {
    if (element->getType() != INT) {
        error->typeError(element->getLine(), name, "integer");
        return false;
    }
    *m_value = ((JsonInt *)element)->getValue();
    if (m_min.has_value() && *m_value < m_min.value()) {
        error->message = "The value for '" + name + "' cannot be less than " +
            std::to_string(m_min.value());
        error->line = element->getLine();
        return false;
    }
    if (m_max.has_value() && *m_value > m_max.value()) {
        error->message = "The value for '" + name +
            "' cannot be greater than " + std::to_string(m_max.value());
        error->line = element->getLine();
        return false;
    }
    return true;
}

JsonBoolReaderStrategy::JsonBoolReaderStrategy(bool *value) {
    m_value = value;
}

bool JsonBoolReaderStrategy::read(
    const std::string &name, JsonElement *element, JsonError *error
) {
    if (element->getType() != BOOLEAN) {
        error->typeError(element->getLine(), name, "boolean");
        return false;
    }
    *m_value = ((JsonBoolean *)element)->getValue();
    return true;
}

JsonStylesReaderStrategy::JsonStylesReaderStrategy(
    StyleManager *styleManager, std::map<std::string, int *> &styles
) {
    m_styleManager = styleManager;
    m_styles = styles;
}

bool JsonStylesReaderStrategy::read(
    const std::string &name, JsonElement *element, JsonError *error
) {
    m_error = error;

    if (element->getType() != OBJECT) {
        error->typeError(element->getLine(), name, "object");
        return false;
    }

    for (auto it : ((JsonObject *)element)->getValue()) {
        auto find = m_styles.find(it.first);
        if (find == m_styles.end()) {
            error->message = "'" + it.first +
                "' is not a recognized style name";
            error->line = it.second.value->getLine();
            return false;
        }

        AnsiStyle style;
        if (!readStyle(it.second.value, &style)) {
            return false;
        }
        *find->second = m_styleManager->add(style);
    }
    return true;
}

bool JsonStylesReaderStrategy::readColorRGB(ColorRGB *color, JsonString *str) {
    if (parseHexColor(str->getValue(), color)) {
        return true;
    }
    m_error->message = "'" + str->getValue() + "' is not a valid hex color";
    m_error->line = str->getLine();
    return false;
}

bool JsonStylesReaderStrategy::readColor16(Color16 *color, JsonString *str) {
    *color = parseColor16(str->getValue());
    if (*color != UNKNOWN) {
        return true;
    }
    m_error->message = "'" + str->getValue() + "' is not a valid color";
    m_error->line = str->getLine();
    return false;
}

bool JsonStylesReaderStrategy::readColor(
    JsonElement *elem, AnsiStyle *style, bool fg
) {
    if (elem->getType() == INT) {
        JsonInt *integer = (JsonInt *)elem;
        int value = integer->getValue();
        if (value < 0 || value > 255) {
            m_error
                ->message = "A color code must be between 0 and 255 inclusive";
            m_error->line = elem->getLine();
            return false;
        }
        fg ? style->fg(value) : style->bg(value);
        return true;
    }
    else if (elem->getType() != STRING) {
        m_error->message = "The datatype of a color must be an int or string";
        m_error->line = elem->getLine();
        return false;
    }

    JsonString *str = (JsonString *)elem;
    if (str->getValue().starts_with("#")) {
        ColorRGB color;
        if (!readColorRGB(&color, str)) {
            return false;
        }
        fg ? style->fg(color) : style->bg(color);
    }
    else {
        Color16 color;
        if (!readColor16(&color, str)) {
            return false;
        }
        fg ? style->fg(color) : style->bg(color);
    }
    return true;
}

bool JsonStylesReaderStrategy::readStyle(
    JsonElement *element, AnsiStyle *style
) {
    if (element->getType() != OBJECT) {
        m_error->message = "The datatype of a style must be an object";
        m_error->line = element->getLine();
        return false;
    }

    std::map<std::string, JsonObjectEntry> obj;
    std::map<std::string, JsonObjectEntry>::const_iterator it;

    obj = ((JsonObject *)element)->getValue();

    for (it = obj.begin(); it != obj.end(); ++it) {
        bool valid;
        if (it->first == "fg") {
            valid = readColor(it->second.value, style, true);
        }
        else if (it->first == "bg") {
            valid = readColor(it->second.value, style, false);
        }
        else if (it->first == "attr") {
            valid = readAttr((JsonArray *)it->second.value, style);
        }
        else {
            m_error->message = "'" + it->first +
                "' is not a recognised style option";
            m_error->line = it->second.key->getLine();
            valid = false;
        }
        if (!valid) {
            return false;
        }
    }
    return true;
}

bool JsonStylesReaderStrategy::readAttr(JsonArray *arr, AnsiStyle *style) {
    if (arr->getType() != ARRAY) {
        m_error->typeError(arr->getLine(), "attr", "array");
        return false;
    }
    auto value = arr->getValue();

    for (int i = 0; i < value.size(); i++) {
        if (value[i]->getType() != STRING) {
            m_error->message = "The datatype of an attr must be a string";
            m_error->line = value[i]->getLine();
            return false;
        }
        std::string attrName = ((JsonString *)value[i])->getValue();

        if (attrName == "bold") {
            style->bold();
        }
        else if (attrName == "italic") {
            style->italic();
        }
        else if (attrName == "underline") {
            style->underline();
        }
        else if (attrName == "curly_underline") {
            style->underline(CURLY);
        }
        else if (attrName == "double_underline") {
            style->underline(DOUBLE);
        }
        else if (attrName == "dotted_underline") {
            style->underline(DOTTED);
        }
        else if (attrName == "dashed_underline") {
            style->underline(DASHED);
        }
        else if (attrName == "blink") {
            style->blink();
        }
        else if (attrName == "reverse") {
            style->standout();
        }
        else {
            m_error->message = "'" + attrName + "' is not a valid attr name";
            m_error->line = value[i]->getLine();
            return false;
        }
    }

    return true;
}

JsonReader::JsonReader(std::map<std::string, JsonReaderStrategy *> &options) {
    m_options = options;
}

const JsonError &JsonReader::getError() const {
    return m_error;
}

bool JsonReader::read(JsonElement *element) {
    if (element->getType() != OBJECT) {
        m_error.message = "Root config must be an object";
        m_error.message = element->getLine();
        return false;
    }

    std::map<std::string, JsonObjectEntry> obj;
    std::map<std::string, JsonObjectEntry>::const_iterator it;

    obj = ((JsonObject *)element)->getValue();

    for (it = obj.begin(); it != obj.end(); it++) {
        auto find = m_options.find(it->first);
        if (find == m_options.end()) {
            m_error.message = "'" + it->first +
                "' is not a recognized config option";
            m_error.line = it->second.key->getLine();
            return false;
        }

        if (!find->second->read(it->first, it->second.value, &m_error)) {
            return false;
        }
    }
    return true;
}
