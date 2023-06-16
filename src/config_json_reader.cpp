#include "../include/util.hpp"
#include "../include/config_json_reader.hpp"

ConfigJsonReader::ConfigJsonReader(StyleManager *styleManager) {
    m_styleManager = styleManager;
}

std::map<std::string, JsonReaderStrategy*> ConfigJsonReader::createOptions() {
    Config &config = Config::instance();

    std::map<std::string, JsonReaderStrategy*> options;
    options["selector"] = new JsonStringReaderStrategy(&config.selector);
    options["active_selector"] =
        new JsonStringReaderStrategy(&config.activeSelector);
    options["prompt"] = new JsonStringReaderStrategy(&config.prompt);

    options["prompt_gap"] = (
            new JsonIntReaderStrategy(&config.promptGap))->min(0);
    options["max_cores"] = (
            new JsonIntReaderStrategy(&config.maxCores))->min(0);
    options["history_limit"] = (
            new JsonIntReaderStrategy(&config.historyLimit))->min(0);
    options["min_hint_spacing"] = (
            new JsonIntReaderStrategy(&config.minHintSpacing))->min(0);
    options["min_hint_width"] = (
            new JsonIntReaderStrategy(&config.minHintWidth))->min(0);
    options["max_hint_width"] = (
            new JsonIntReaderStrategy(&config.maxHintWidth))->min(0);
    options["show_spinner"] = (
            new JsonBoolReaderStrategy(&config.showSpinner));

     options["matcher"] = new JsonEnumReaderStrategy<MatcherType>(
         &config.matcher,
         std::map<std::string, MatcherType>{
             {"fuzzy", FUZZY_MATCHER},
             {"regex", REGEX_MATCHER},
             {"exact", REGEX_MATCHER}}
     );
     options["case_mode"] = new JsonEnumReaderStrategy<CaseSensitivity>(
         &config.caseSensitivity,
         std::map<std::string, CaseSensitivity>{
            {"sensitive", CASE_SENSITIVE},
            {"insensitive", CASE_INSENSITIVE},
            {"smart", SMART_CASE}}
     );

    std::map<std::string, int*> styles;
    styles["item"] = &config.itemStyle;
    styles["active_item"] = &config.activeItemStyle;
    styles["hint"] = &config.hintStyle;
    styles["active_hint"] = &config.activeHintStyle;
    styles["selector"] = &config.selectorStyle;
    styles["active_selector"] = &config.activeSelectorStyle;
    styles["active_row"] = &config.activeRowStyle;
    styles["row"] = &config.rowStyle;
    styles["search_prompt"] = &config.searchPromptStyle;
    styles["search"] = &config.searchStyle;
    styles["search_row"] = &config.searchRowStyle;
    styles["background"] = &config.backgroundStyle;
    options["style"] = new JsonStylesReaderStrategy(m_styleManager, styles);

    return options;
}

const JsonError ConfigJsonReader::getError() const {
    return m_error;
}

bool ConfigJsonReader::readJsonFile(JsonParser &parser, std::ifstream& ifs) {
    if (!ifs.is_open()) {
        m_error.message = "File was not open for reading";
        m_error.line = 0;
        return false;
    }

    std::string json((std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()));
    if (!parser.parse(json)) {
        m_error.message = parser.getError();
        m_error.line = parser.getLine();
        return false;
    }

    return true;
}

bool ConfigJsonReader::read(std::ifstream& ifs) {
    JsonParser parser;

    if (!readJsonFile(parser, ifs)) {
        return false;
    }
    JsonElement *root = parser.getElement();

    if (root == nullptr) {
        return true;
    }

    std::map<std::string, JsonReaderStrategy*> options = createOptions();

    JsonReader reader(options);
    bool success = reader.read(root);

    for (const auto &it : options) {
        delete it.second;
    }

    if (!success) {
        m_error = reader.getError();
        return false;
    }


    return true;
}
