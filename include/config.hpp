#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ansi_style.hpp"
#include "case_sensitivity.hpp"
#include "item_matcher.hpp"
#include "style_manager.hpp"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

enum class Placement {
    Top,
    Bottom,
    Left,
    Right,
};

enum class VerticalPlacement {
    Top,
    Bottom,
};

enum BorderChar {
    HORIZONTAL_BORDER,
    VERTICAL_BORDER,
    TOP_LEFT_BORDER,
    TOP_RIGHT_BORDER,
    BOTTOM_RIGHT_BORDER,
    BOTTOM_LEFT_BORDER,
    TOP_JOIN_BORDER,
    RIGHT_JOIN_BORDER,
    BOTTOM_JOIN_BORDER,
    LEFT_JOIN_BORDER,
    FULL_JOIN_BORDER
};

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
    int borderStyle = NO_STYLE;
    int previewLineStyle = NO_STYLE;

    bool externalBorder;
    bool previewBorder;
    bool itemsBorder;
    bool queryBorder;
    bool queryWindow;

    MatcherType matcher = FUZZY_MATCHER;
    CaseSensitivity caseSensitivity = CASE_SENSITIVE;

    std::string prompt = ">";
    int promptGap = 1;
    std::string selector = "  ";
    std::string activeSelector = "* ";
    std::string query = "";
    std::string logFile = "";
    std::string command = "";
    std::string preview = "";
    std::string previewLine = "";
    std::vector<std::string> borderChars{"─", "│", "┌", "┐", "┘", "└",
                                         "┬", "┤", "┴", "├", "┼"};

    float previewPercent = 0.5f;
    Placement previewPlacement = Placement::Top;
    VerticalPlacement queryPlacement = VerticalPlacement::Bottom;
    int tabstop = 8;

    bool showHelp = false;
    bool showHints = false;
    bool selectHint = false;
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

void createStyles();
bool readConfig(int argc, const char **argv);

#endif
