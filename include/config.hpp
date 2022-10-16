#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <filesystem>
#include "ansi_style.hpp"

namespace fs = std::filesystem;

struct Config
{
    std::string selectedSourceStyle = AnsiStyle().fg(WHITE).build();
    std::string selectedHintStyle   = selectedSourceStyle;
    std::string sourceStyle         = AnsiStyle().fg(BLUE).build();
    std::string hintStyle           = AnsiStyle().dim().build();
    std::string backgroundStyle     = AnsiStyle().build();
    std::string rowStyle            = backgroundStyle;
    std::string selectedRowStyle    = backgroundStyle;
    std::string selectorStyle       = backgroundStyle;
    std::string searchPromptStyle   = backgroundStyle;
    std::string searchStyle         = backgroundStyle;
    std::string searchRowStyle      = backgroundStyle;

    std::string prompt             = ">";
    std::string selector           = "* ";

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
