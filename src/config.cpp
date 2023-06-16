#include "../include/config.hpp"
#include "../include/config_json_reader.hpp"
#include "../include/config_option_reader.hpp"
#include "../include/util.hpp"

void createStyles(StyleManager *styleManager) {
    Config &config = Config::instance();
    if (config.itemStyle == NO_STYLE) {
        config.itemStyle = styleManager->add(AnsiStyle().fg(BLUE));
    }
    if (config.hintStyle == NO_STYLE) {
        config.hintStyle = styleManager->add(AnsiStyle().fg(BRIGHT_BLACK));
    }
    if (config.activeItemStyle == NO_STYLE) {
        config.activeItemStyle = styleManager->add(AnsiStyle().fg(WHITE));
    }
    if (config.activeHintStyle == NO_STYLE) {
        config.activeHintStyle = styleManager->add(AnsiStyle().fg(WHITE));
    }
}

bool readConfig(StyleManager *styleManager, int argc, const char **argv) {
    ConfigJsonReader configJsonReader(styleManager);
    ConfigOptionReader configOptionReader;

    std::ifstream ifs(expandUserPath("~/.config/jfind/config.json"));
    if (ifs.is_open()) {
        if (!configJsonReader.read(ifs)) {
            fprintf(
                stderr, "Error in config.json on line %d: %s\n",
                configJsonReader.getError().line,
                configJsonReader.getError().message.c_str()
            );
            return false;
        }
    }

    if (!configOptionReader.read(argc, argv)) {
        return false;
    }

    return true;
}
