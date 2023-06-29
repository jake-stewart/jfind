#include "../include/config_json_reader.hpp"
#include "../include/util.hpp"

std::map<std::string, JsonReaderStrategy *> ConfigJsonReader::createOptions() {
    Config &config = Config::instance();
    std::map<std::string, JsonReaderStrategy *> options;

    options["selector"]
        = new JsonStringReaderStrategy(&config.selector);
    options["active_selector"]
        = new JsonStringReaderStrategy(&config.activeSelector);
    options["prompt"]
        = new JsonStringReaderStrategy(&config.prompt);
    options["prompt_gap"]
        = (new JsonIntReaderStrategy(&config.promptGap))->min(0);
    options["max_cores"]
        = (new JsonIntReaderStrategy(&config.maxCores))->min(0);
    options["tabstop"]
        = (new JsonIntReaderStrategy(&config.tabstop))->min(0)->max(8);
    options["history_limit"]
        = (new JsonIntReaderStrategy(&config.historyLimit))->min(0);
    options["min_hint_spacing"]
        = (new JsonIntReaderStrategy(&config.minHintSpacing))->min(0);
    options["min_hint_width"]
        = (new JsonIntReaderStrategy(&config.minHintWidth))->min(0);
    options["max_hint_width"]
        = (new JsonIntReaderStrategy(&config.maxHintWidth))->min(0);
    options["show_spinner"]
        = (new JsonBoolReaderStrategy(&config.showSpinner));
    options["border_chars"]
        = (new JsonStringArrayReaderStrategy(&config.borderChars))
            ->min(11)->max(11);
    options["wrap_item_list"]
        = (new JsonBoolReaderStrategy(&config.wrapItemList));

    options["window_style"] = new JsonEnumReaderStrategy(
        &config.windowStyle, {
            {"compact", WindowStyle::Compact},
            {"merged", WindowStyle::Merged},
            {"windowed", WindowStyle::Windowed}
        }
    );

    options["matcher"] = new JsonEnumReaderStrategy(
        &config.matcher, {
            {"fuzzy", FUZZY_MATCHER},
            {"regex", REGEX_MATCHER},
            {"exact", REGEX_MATCHER}}
    );

    options["preview_position"] = new JsonEnumReaderStrategy(
        &config.previewPlacement, {
            {"top", Placement::Top},
            {"bottom", Placement::Bottom},
            {"left", Placement::Left},
            {"right", Placement::Right}
        }
    );

    options["case_mode"] = new JsonEnumReaderStrategy(
        &config.caseSensitivity, {
            {"sensitive", CASE_SENSITIVE},
            {"insensitive", CASE_INSENSITIVE},
            {"smart", SMART_CASE}
        }
    );

    std::map<std::string, JsonStyleContext> styles{
        {"item", {&config.itemStyle}},
        {"active_item", {&config.activeItemStyle}},
        {"hint", {&config.hintStyle}},
        {"active_hint", {&config.activeHintStyle}},
        {"selector", {&config.selectorStyle}},
        {"active_selector", {&config.activeSelectorStyle}},
        {"active_row", {&config.activeRowStyle}},
        {"row", {&config.rowStyle}},
        {"search_prompt", {&config.searchPromptStyle}},
        {"search", {&config.searchStyle}},
        {"search_row", {&config.searchRowStyle}},
        {"background", {&config.backgroundStyle}},
        {"border", {&config.borderStyle}},
        {"preview_line", {&config.previewLineStyle, true}},
    };
    options["style"] = new JsonStylesReaderStrategy(styles);

    return options;
}

const JsonError ConfigJsonReader::getError() const {
    return m_error;
}

bool ConfigJsonReader::readJsonFile(JsonParser &parser, std::ifstream &ifs) {
    if (!ifs.is_open()) {
        m_error.message = "File was not open for reading";
        m_error.line = 0;
        return false;
    }

    std::string json(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>())
    );
    if (!parser.parse(json)) {
        m_error.message = parser.getError();
        m_error.line = parser.getLine();
        return false;
    }

    return true;
}

bool ConfigJsonReader::read(std::ifstream &ifs) {
    JsonParser parser;

    if (!readJsonFile(parser, ifs)) {
        return false;
    }
    JsonElement *root = parser.getElement();

    if (root == nullptr) {
        return true;
    }

    std::map<std::string, JsonReaderStrategy *> options = createOptions();

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
