#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <filesystem>
#include "ansi_style.hpp"
#include "style_manager.hpp"

namespace fs = std::filesystem;

struct Config
{
    int activeSourceStyle = NO_STYLE;
    int activeHintStyle = NO_STYLE;
    int sourceStyle = NO_STYLE;
    int hintStyle = NO_STYLE;
    int backgroundStyle = NO_STYLE;
    int rowStyle = NO_STYLE;
    int activeRowStyle = NO_STYLE;
    int selectorStyle = NO_STYLE;
    int activeSelectorStyle = NO_STYLE;
    int searchPromptStyle = NO_STYLE;
    int searchStyle = NO_STYLE;
    int searchRowStyle = NO_STYLE;

    std::string prompt             = ">";
    int promptGap                  = 1;
    std::string selector           = "  ";
    std::string activeSelector     = "* ";

    std::string query              = "";

    bool        showHints          = false;
    bool        selectHint         = false;
    bool        selectBoth         = false;
    bool        acceptNonMatch     = false;
    fs::path    sourceFile;

    fs::path    historyFile;
    int         historyCount = 50;

    int minInfoWidth = 25;
    int minInfoSpacing = 5;
    int maxInfoWidth = 60;
};

#endif
