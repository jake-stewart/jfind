#include "../include/util.hpp"
#include "../include/json_reader.hpp"

std::string errorMsg;
int errorLine = 0;

void typeError(std::string name, std::string type) {
    errorMsg = "The datatype for '" + name + "' must be "
        + (isVowel(type[0]) ? "an" : "a") + " " + type;
}

JsonStringReaderStrategy::JsonStringReaderStrategy(std::string *value) {
    m_value = value;
}

JsonStringReaderStrategy* JsonStringReaderStrategy::min(int minLen) {
    m_minLen = minLen;
    return this;
}

JsonStringReaderStrategy* JsonStringReaderStrategy::max(int maxLen) {
    m_maxLen = maxLen;
    return this;
}

bool JsonStringReaderStrategy::read(const std::string& name,
        JsonElement *element)
{
    if (element->getType() != STRING) {
        typeError(name, "string");
        errorLine = element->getLine();
        return false;
    }
    *m_value = ((JsonString*)element)->getValue();
    if (m_minLen.has_value() && m_value->length() < m_minLen.value()) {
        errorMsg = "The value for '" + name
            + "' cannot have a length less than "
            + std::to_string(m_minLen.value());
        errorLine = element->getLine();
        return false;
    }
    if (m_maxLen.has_value() && m_value->length() > m_maxLen.value()) {
        errorMsg = "The value for '" + name
            + "' cannot have a length greater than "
            + std::to_string(m_maxLen.value());
        errorLine = element->getLine();
        return false;
    }
    return true;
}

JsonIntReaderStrategy::JsonIntReaderStrategy(int *value) {
    m_value = value;
}

JsonIntReaderStrategy* JsonIntReaderStrategy::min(int min) {
    m_min = min;
    return this;
}

JsonIntReaderStrategy* JsonIntReaderStrategy::max(int max) {
    m_max = max;
    return this;
}

bool JsonIntReaderStrategy::read(const std::string& name, JsonElement *element)
{
    if (element->getType() != INT) {
        typeError(name, "integer");
        errorLine = element->getLine();
        return false;
    }
    *m_value = ((JsonInt*)element)->getValue();
    if (m_min.has_value() && *m_value < m_min.value()) {
        errorMsg = "The value for '" + name + "' cannot be less than "
            + std::to_string(m_min.value());
        errorLine = element->getLine();
        return false;
    }
    if (m_max.has_value() && *m_value > m_max.value()) {
        errorMsg = "The value for '" + name + "' cannot be greater than "
            + std::to_string(m_max.value());
        errorLine = element->getLine();
        return false;
    }
    return true;
}

JsonStylesReaderStrategy::JsonStylesReaderStrategy(StyleManager *styleManager,
        std::map<std::string, int*>& styles)
{
    m_styleManager = styleManager;
    m_styles = styles;
}

bool JsonStylesReaderStrategy::read(const std::string& name,
        JsonElement *element)
{
    if (element->getType() != OBJECT) {
        typeError(name, "object");
        errorLine = element->getLine();
        return false;
    }

    for (auto it : ((JsonObject*)element)->getValue()) {
        auto find = m_styles.find(it.first);
        if (find == m_styles.end()) {
            errorMsg = "'" + it.first + "' is not a recognized style name";
            errorLine = it.second.value->getLine();
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
    errorMsg = "'" + str->getValue() + "' is not a valid hex color";
    errorLine = str->getLine();
    return false;
}

bool JsonStylesReaderStrategy::readColor16(Color16 *color, JsonString *str) {
    *color = parseColor16(str->getValue());
    if (*color != UNKNOWN) {
        return true;
    }
    errorMsg = "'" + str->getValue() + "' is not a valid color";
    errorLine = str->getLine();
    return false;
}

bool JsonStylesReaderStrategy::readColor(JsonString *str, AnsiStyle *style,
        bool fg)
{
    if (str->getType() != STRING) {
        errorMsg = "The datatype of a color must be a string";
        errorLine = str->getLine();
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

bool JsonStylesReaderStrategy::readStyle(JsonElement *element,
        AnsiStyle *style)
{
    if (element->getType() != OBJECT) {
        errorMsg = "The datatype of a style must be an object";
        errorLine = element->getLine();
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
            errorMsg = "'" + it->first + "' is not a recognised style option";
            errorLine = it->second.key->getLine();
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
            errorMsg = "The datatype of a color attr must be a string";
            errorLine = value[i]->getLine();
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
            errorMsg = "'" + attrName + "' is not a valid attr name";
            errorLine = value[i]->getLine();
            return false;
        }
    }
    
    return true;
}

JsonReader::JsonReader(std::map<std::string, JsonReaderStrategy*>& options) {
    m_options = options;
}

std::string& JsonReader::getError() {
    return errorMsg;
}

int JsonReader::getErrorLine() {
    return errorLine;
}

bool JsonReader::read(JsonElement *element) {
    if (element->getType() != OBJECT) {
        errorMsg = "Root config must be an object";
        errorLine = element->getLine();
        return false;
    }

    std::map<std::string, JsonObjectEntry> obj;
    std::map<std::string, JsonObjectEntry>::const_iterator it;

    obj = ((JsonObject*)element)->getValue();

    for (it = obj.begin(); it != obj.end(); it++) {
        auto find = m_options.find(it->first);
        if (find == m_options.end()) {
            errorMsg = "'" + it->first
                + "' is not a recognized config option";
            errorLine = it->second.key->getLine();
            return false;
        }

        if (!find->second->read(it->first, it->second.value)) {
            return false;
        }
    }
    return true;
}
