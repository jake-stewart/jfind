#include "../include/util.hpp"
#include "../include/json_reader.hpp"

std::string error_msg = "None";
int error_line = 0;

void typeError(std::string name, std::string type) {
    error_msg = "The datatype for '" + name + "' must be "
        + (isVowel(type[0]) ? "an" : "a") + type;
}

JsonStringReaderStrategy::JsonStringReaderStrategy(std::string *value) {
    m_value = value;
}

JsonStringReaderStrategy* JsonStringReaderStrategy::min(int length) {
    m_min_len = length;
    return this;
}

JsonStringReaderStrategy* JsonStringReaderStrategy::max(int length) {
    m_max_len = length;
    return this;
}

bool JsonStringReaderStrategy::read(
        const std::string& name,
        JsonElement *element
) {
    if (element->getType() != STRING) {
        typeError(name, "string");
        error_line = element->getLine();
        return false;
    }
    *m_value = ((JsonString*)element)->getValue();
    if (m_min_len.has_value() && m_value->length() < m_min_len.value()) {
        error_msg = "The value for '" + name
            + "' cannot have a length less than "
            + std::to_string(m_min_len.value());
        error_line = element->getLine();
        return false;
    }
    if (m_max_len.has_value() && m_value->length() > m_max_len.value()) {
        error_msg = "The value for '" + name
            + "' cannot have a length greater than "
            + std::to_string(m_max_len.value());
        error_line = element->getLine();
        return false;
    }
    return true;
}

JsonIntReaderStrategy::JsonIntReaderStrategy(int *value) {
    m_value = value;
}

JsonIntReaderStrategy* JsonIntReaderStrategy::min(int length) {
    m_min_len = length;
    return this;
}

JsonIntReaderStrategy* JsonIntReaderStrategy::max(int length) {
    m_max_len = length;
    return this;
}

bool JsonIntReaderStrategy::read(
        const std::string& name,
        JsonElement *element
) {
    if (element->getType() != INT) {
        typeError(name, "integer");
        error_line = element->getLine();
        return false;
    }
    *m_value = ((JsonInt*)element)->getValue();
    if (m_min_len.has_value() && *m_value < m_min_len.value()) {
        error_msg = "The value for '" + name + "' cannot be less than "
            + std::to_string(m_min_len.value());
        error_line = element->getLine();
        return false;
    }
    if (m_max_len.has_value() && *m_value > m_max_len.value()) {
        error_msg = "The value for '" + name + "' cannot be greater than "
            + std::to_string(m_min_len.value());
        error_line = element->getLine();
        return false;
    }
    return true;
}

JsonStylesReaderStrategy::JsonStylesReaderStrategy(
        StyleManager *style_manager,
        std::map<std::string, int*>& styles
) {
    m_style_manager = style_manager;
    m_styles = styles;
}

bool JsonStylesReaderStrategy::read(
        const std::string& name,
        JsonElement *element
) {
    if (element->getType() != OBJECT) {
        typeError(name, "object");
        error_line = element->getLine();
        return false;
    }

    for (auto it : ((JsonObject*)element)->getValue()) {
        auto find = m_styles.find(it.first);
        if (find == m_styles.end()) {
            error_msg = "'" + it.first + "' is not a recognized style name";
            error_line = it.second.value->getLine();
            return false;
        }

        AnsiStyle style;
        if (!readStyle(it.second.value, &style)) {
            return false;
        }
        *find->second = m_style_manager->add(style);
    }
    return true;
}


bool JsonStylesReaderStrategy::readColorRGB(ColorRGB *color, JsonString *str) {
    if (parseHexColor(str->getValue(), color)) {
        return true;
    }
    error_msg = "'" + str->getValue() + "' is not a valid hex color";
    error_line = str->getLine();
    return false;
}

bool JsonStylesReaderStrategy::readColor16(Color16 *color, JsonString *str) {
    *color = parseColor16(str->getValue());
    if (*color != UNKNOWN) {
        return true;
    }
    error_msg = "'" + str->getValue() + "' is not a valid color";
    error_line = str->getLine();
    return false;
}

bool JsonStylesReaderStrategy::readColor(
        JsonString *str,
        AnsiStyle *style,
        bool fg
) {
    if (str->getType() != STRING) {
        error_msg = "The datatype of a color must be a string";
        error_line = str->getLine();
        return false;
    }
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
        JsonElement *element,
        AnsiStyle *style
) {
    if (element->getType() != OBJECT) {
        error_msg = "The datatype of a style must be an object";
        error_line = element->getLine();
        return false;
    }

    std::map<std::string, JsonObjectEntry> obj;
    std::map<std::string, JsonObjectEntry>::const_iterator it;

    obj = ((JsonObject*)element)->getValue();

    for (it = obj.begin(); it != obj.end(); ++it) {
        bool valid;
        if (it->first == "fg") {
            valid = readColor((JsonString*)it->second.value, style, true);
        }
        else if (it->first == "bg") {
            valid = readColor((JsonString*)it->second.value, style, false);
        }
        else if (it->first == "attr") {
            valid = readAttr((JsonArray*)it->second.value, style);
        }
        else {
            error_msg = "'" + it->first + "' is not a recognised style option";
            error_line = it->second.key->getLine();
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
        typeError("attr", "array");
        return false;
    }
    auto value = arr->getValue();

    for (int i = 0; i < value.size(); i++) { 
        if (value[i]->getType() != STRING) {
            error_msg = "The datatype of a color attr must be a string";
            error_line = value[i]->getLine();
            return false;
        }
        std::string attrName = ((JsonString*)value[i])->getValue();

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
            error_msg = "'" + attrName + "' is not a valid attr name";
            error_line = value[i]->getLine();
            return false;
        }
    }
    
    return true;
}

JsonReader::JsonReader(std::map<std::string, JsonReaderStrategy*>& options) {
    m_options = options;
}

std::string& JsonReader::getError() {
    return error_msg;
}

int JsonReader::getErrorLine() {
    return error_line;
}

bool JsonReader::read(JsonElement *element) {
    if (element->getType() != OBJECT) {
        error_msg = "Root config must be an object";
        error_line = element->getLine();
        return false;
    }

    std::map<std::string, JsonObjectEntry> obj;
    std::map<std::string, JsonObjectEntry>::const_iterator it;

    obj = ((JsonObject*)element)->getValue();

    for (it = obj.begin(); it != obj.end(); it++) {
        auto find = m_options.find(it->first);
        if (find == m_options.end()) {
            error_msg = "'" + it->first
                + "' is not a recognized config option";
            error_line = it->second.key->getLine();
            return false;
        }

        if (!find->second->read(it->first, it->second.value)) {
            return false;
        }
    }
    return true;
}
