#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <filesystem>
#include "ansi_style.hpp"
#include "style_manager.hpp"

namespace fs = std::filesystem;

struct Config {
    int activeItemStyle = NO_STYLE;
    int activeHintStyle = NO_STYLE;
    int itemStyle = NO_STYLE;
    int hintStyle = NO_STYLE;
    int backgroundStyle = NO_STYLE;
    int rowStyle = NO_STYLE;
    int activeRowStyle = NO_STYLE;
    int selectorStyle = NO_STYLE;
    int activeSelectorStyle = NO_STYLE;
    int searchPromptStyle = NO_STYLE;
    int searchStyle = NO_STYLE;
    int searchRowStyle = NO_STYLE;

    std::string prompt = ">";
    int promptGap = 1;
    std::string selector = "  ";
    std::string activeSelector = "* ";
    std::string query = "";

    bool showHelp = false;
    bool showHints = false;
    bool selectHint = false;
    bool selectBoth = false;
    bool showSpinner = true;
    bool acceptNonMatch = false;

    fs::path historyFile;
    int historyLimit = 50;

    int minHintWidth = 25;
    int minHintSpacing = 5;
    int maxHintWidth = 60;
};

#endif
