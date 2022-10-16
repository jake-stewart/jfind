#include <fstream>
#include "../include/util.hpp"
#include "../include/config_json_reader.hpp"

#define ASSERT_TYPE(ELEMENT, TYPE, ERROR) \
    if (ELEMENT->getType() != TYPE) { \
        errorLine = ELEMENT->getLine(); \
        error = ERROR; \
        return false; \
    }

#define READ_STYLE(KEY, VAR) \
    if (it->first == KEY) { \
        if (!readStyle((JsonObject*)it->second.value, &style)) { \
            return false; \
        } \
        config->VAR = style.build(); \
        continue; \
    }

bool ConfigJsonReader::read(const char* file, Config *config) {
    this->config = config;

    std::ifstream ifs(expandUserPath(file));
    if (!ifs.is_open()) {
        return true;
    }

    std::string json(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>())
    );
    JsonParser jsonParser;
    if (!jsonParser.parse(json)) {
        error = jsonParser.getError();
        errorLine = jsonParser.getLine();
        return false;
    }

    if (jsonParser.getElement() == nullptr) {
        return true;
    }

    return readRoot((JsonObject*)jsonParser.getElement());
}

bool ConfigJsonReader::readRoot(JsonObject *obj) {
    ASSERT_TYPE(obj, OBJECT, "The root config must be an object");

    auto value = obj->getValue();
    for (auto it = value.begin(); it != value.end(); ++it) { 
        if (it->first == "prompt") {
            ASSERT_TYPE(
                it->second.value,
                STRING,
                "The value for 'prompt' must be a string"
            );
            config->prompt = ((JsonString*)it->second.value)->getValue();
        }
        else if (it->first == "selector") {
            ASSERT_TYPE(
                it->second.value,
                STRING,
                "The value for 'selector' must be a string"
            );
            config->selector = ((JsonString*)it->second.value)->getValue();
        }
        else if (it->first == "style") {
            if (!readStyles((JsonObject*)it->second.value)) {
                return false;
            }
        }
        else if (it->first == "history-count") {
            ASSERT_TYPE(
                it->second.value,
                INT,
                "The value for 'history-count' must be an integer"
            );
            int value = ((JsonInt*)it->second.value)->getValue();
            if (value < 0) {
                error = "The value 'history-count' cannot be negative";
                errorLine = it->second.value->getLine();
                return false;
            }
            config->historyCount = value;
        }
        else if (it->first == "min-info-spacing") {
            ASSERT_TYPE(
                it->second.value,
                INT,
                "The value for 'min-info-spacing' must be an integer"
            );
            int value = ((JsonInt*)it->second.value)->getValue();
            if (value < 0) {
                error = "The value 'min-info-spacing' cannot be negative";
                errorLine = it->second.value->getLine();
                return false;
            }
            config->minInfoSpacing = value;
        }
        else if (it->first == "min-info-width") {
            ASSERT_TYPE(
                it->second.value,
                INT,
                "The value for 'min-info-width' must be an integer"
            );
            int value = ((JsonInt*)it->second.value)->getValue();
            if (value < 0) {
                error = "The value 'min-info-width' cannot be negative";
                errorLine = it->second.value->getLine();
                return false;
            }
            config->minInfoWidth = value;
        }
        else if (it->first == "max-info-width") {
            ASSERT_TYPE(
                it->second.value,
                INT,
                "The value for 'max-info-width' must be an integer"
            );
            int value = ((JsonInt*)it->second.value)->getValue();
            if (value < 2) {
                error = "The value 'max-info-width' less than 2";
                errorLine = it->second.value->getLine();
                return false;
            }
            config->maxInfoWidth = value;
        }
        else {
            error = "Unknown key: '" + it->first + "'";
            errorLine = it->second.key->getLine();
            return false;
        }
    }

    return true;
}

bool ConfigJsonReader::readStyles(JsonObject *obj) {
    ASSERT_TYPE(obj, OBJECT, "The value for 'styles' must be an object");

    auto value = obj->getValue();
    for (auto it = value.begin(); it != value.end(); ++it) { 
        AnsiStyle style;
        READ_STYLE("source", sourceStyle);
        READ_STYLE("selected-source", selectedSourceStyle);
        READ_STYLE("hint", hintStyle);
        READ_STYLE("selected-hint", selectedHintStyle);
        READ_STYLE("selector", selectorStyle);
        READ_STYLE("selected-row", selectedRowStyle);
        READ_STYLE("row", rowStyle);
        READ_STYLE("search-prompt", searchPromptStyle);
        READ_STYLE("search", searchStyle);
        READ_STYLE("search-row", searchRowStyle);
        READ_STYLE("background", backgroundStyle);

        error = "'" + it->first + "' is not a recognized style name";
        errorLine = it->second.key->getLine();
        return false;
    }
    return true;
}

bool ConfigJsonReader::readStyle(JsonObject *obj, AnsiStyle *style) {
    ASSERT_TYPE(obj, OBJECT, "The value for a style must be an object");

    auto value = obj->getValue();
    for (auto it = value.begin(); it != value.end(); ++it) { 
        if (it->first == "fg") {
            if (it->second.value->getType() != STRING) {
                error = "The value for a color must be a string";
                errorLine = it->second.value->getLine();
                return false;
            }
            std::string colorName =
                ((JsonString*)it->second.value)->getValue();
            if (colorName.starts_with("#")) {
                ColorRGB color;
                std::string hex = colorName.substr(1);
                if (!parseHexColor(hex, &color)) {
                    error = "'" + colorName + "' is not a valid hex color";
                    errorLine = it->second.value->getLine();
                    return false;
                }
                style->fg(color);
            }
            else {
                Color16 color = parseColor16(colorName);
                if (color == UNKNOWN) {
                    error = "'" + colorName + "' is not a valid color";
                    errorLine = it->second.value->getLine();
                    return false;
                }
                style->fg(color);
            }
        }
        else if (it->first == "bg") {
            if (it->second.value->getType() != STRING) {
                error = "The value for a color must be a string";
                errorLine = it->second.value->getLine();
                return false;
            }
            std::string colorName =
                ((JsonString*)it->second.value)->getValue();
            if (colorName.starts_with("#")) {
                ColorRGB color;
                std::string hex = colorName.substr(1);
                if (!parseHexColor(hex, &color)) {
                    error = "'" + colorName + "' is not a valid hex color";
                    errorLine = it->second.value->getLine();
                    return false;
                }
                style->bg(color);
            }
            else {
                Color16 color = parseColor16(colorName);
                if (color == UNKNOWN) {
                    error = "'" + colorName + "' is not a valid color name";
                    errorLine = it->second.value->getLine();
                    return false;
                }
                style->bg(color);
            }
        }
        else if (it->first == "attr") {
            if (!readAttr((JsonArray*)it->second.value, style)) {
                return false;
            }
        }
        else {
            error = "Unknown key for style: '" + it->first + "'";
            errorLine = it->second.key->getLine();
            return false;
        }
    }
    return true;
}

Color16 ConfigJsonReader::readColor(JsonString *str) {
    if (str->getType() != STRING) {
        error = "The value for a color must be a string";
        errorLine = str->getLine();
        return UNKNOWN;
    }
    Color16 color = parseColor16(str->getValue());
    if (color == UNKNOWN) {
        errorLine = str->getLine();
        error = "'" + str->getValue() + "' is not a valid color name";
    }
    return color;
}

bool ConfigJsonReader::readAttr(JsonArray *arr, AnsiStyle *style) {
    ASSERT_TYPE(arr, ARRAY, "The value for style attrs must be an array");
    auto value = arr->getValue();

    for (int i = 0; i < value.size(); i++) { 
        ASSERT_TYPE(
            value[i],
            STRING,
            "The value for a color attr must be a string"
        );
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
            error = "'" + attrName + "' is not a valid attr name";
            errorLine = value[i]->getLine();
            return false;
        }
    }
    
    return true;
}

std::string ConfigJsonReader::getError() {
    return error;
}

int ConfigJsonReader::getErrorLine() {
    return errorLine;
}
