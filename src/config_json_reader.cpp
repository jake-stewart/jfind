#include <fstream>
#include "../include/util.hpp"
#include "../include/config_json_reader.hpp"

std::map<std::string, JsonReaderStrategy*> ConfigJsonReader::createOptions() {
    std::map<std::string, JsonReaderStrategy*> options;
    options["selector"]         =  new JsonStringReaderStrategy(&m_config->selector);
    options["active-selector"]  =  new JsonStringReaderStrategy(&m_config->activeSelector);
    options["prompt"]           =  new JsonStringReaderStrategy(&m_config->prompt);

    options["prompt-gap"]       = (new JsonIntReaderStrategy(&m_config->promptGap))     ->min(0);
    options["history-count"]    = (new JsonIntReaderStrategy(&m_config->historyCount))  ->min(0);
    options["min-info-spacing"] = (new JsonIntReaderStrategy(&m_config->minInfoSpacing))->min(0);
    options["min-info-width"]   = (new JsonIntReaderStrategy(&m_config->minInfoWidth))  ->min(0);
    options["max-info-width"]   = (new JsonIntReaderStrategy(&m_config->maxInfoWidth))  ->min(0);

    std::map<std::string, int*> styles;
    styles["source"]          = &m_config->sourceStyle;
    styles["active-source"]   = &m_config->activeSourceStyle;
    styles["hint"]            = &m_config->hintStyle;
    styles["active-hint"]     = &m_config->activeHintStyle;
    styles["selector"]        = &m_config->selectorStyle;
    styles["active-selector"] = &m_config->activeSelectorStyle;
    styles["active-row"]      = &m_config->activeRowStyle;
    styles["row"]             = &m_config->rowStyle;
    styles["search-prompt"]   = &m_config->searchPromptStyle;
    styles["search"]          = &m_config->searchStyle;
    styles["search-row"]      = &m_config->searchRowStyle;
    styles["background"]      = &m_config->backgroundStyle;
    options["style"] = new JsonStylesReaderStrategy(m_style_manager, styles);

    return options;
}

std::string& ConfigJsonReader::getError() {
    return m_error;
}

int ConfigJsonReader::getErrorLine() {
    return m_error_line;
}

ConfigJsonReader::ConfigJsonReader(Config *config, StyleManager *style_manager) {
    m_config = config;
    m_style_manager = style_manager;
}

bool ConfigJsonReader::readJsonFile(JsonElement **root, const char *file) {
    std::ifstream ifs(expandUserPath(file));

    if (!ifs.is_open()) {
        root = nullptr;
        return true;
    }

    std::string json(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>())
    );
    JsonParser jsonParser;
    if (!jsonParser.parse(json)) {
        m_error = jsonParser.getError();
        m_error_line = jsonParser.getLine();
        return false;
    }

    *root = jsonParser.getElement();
    return true;
}

bool ConfigJsonReader::read(const char *file) {
    JsonElement *root;
    if (!readJsonFile(&root, file)) {
        return false;
    }

    if (root == nullptr) {
        return true;
    }

    std::map<std::string, JsonReaderStrategy*> options = createOptions();

    JsonReader reader(options);
    if (!reader.read(root)) {
        m_error = reader.getError();
        m_error_line = reader.getErrorLine();
        return false;
    }

    return true;
}
