#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ansi_style.hpp"
#include "case_sensitivity.hpp"
#include "item_matcher.hpp"
#include "style_manager.hpp"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

struct Config
{
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

    MatcherType matcher = FUZZY_MATCHER;
    CaseSensitivity caseSensitivity = CASE_SENSITIVE;

    std::string prompt = ">";
    int promptGap = 1;
    std::string selector = "  ";
    std::string activeSelector = "* ";
    std::string query = "";
    std::string logFile = "";
    std::string command = "";

    bool showHelp = false;
    bool showHints = false;
    bool selectHint = false;
    bool selectBoth = false;
    bool showSpinner = true;
    bool acceptNonMatch = false;

    fs::path historyFile;
    int historyLimit = 50;

    int maxCores = 4;
    int minHintWidth = 25;
    int minHintSpacing = 5;
    int maxHintWidth = 60;

    std::vector<int> additionalKeys;
    bool showKey = false;

    static Config &instance() {
        static Config singleton;
        return singleton;
    }

private:
    Config(){};
};

void createStyles(StyleManager *styleManager);
bool readConfig(StyleManager *styleManager, int argc, const char **argv);

#endif
