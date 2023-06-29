#include "../include/config.hpp"
#include "../include/config_json_reader.hpp"
#include "../include/config_option_reader.hpp"
#include "../include/util.hpp"

void createStyles() {
    Config &config = Config::instance();
    if (config.itemStyle == NO_STYLE) {
        config.itemStyle = StyleManager::instance().add(
            AnsiStyle().fg(BLUE)
        );
    }
    if (config.hintStyle == NO_STYLE) {
        config.hintStyle = StyleManager::instance().add(
            AnsiStyle().fg(BRIGHT_BLACK)
        );
    }
    if (config.activeItemStyle == NO_STYLE) {
        config.activeItemStyle = StyleManager::instance().add(
            AnsiStyle().fg(WHITE)
        );
    }
    if (config.activeHintStyle == NO_STYLE) {
        config.activeHintStyle = StyleManager::instance().add(
            AnsiStyle().fg(WHITE)
        );
    }
    if (config.borderStyle == NO_STYLE) {
        config.borderStyle = StyleManager::instance().add(
            AnsiStyle().fg(BRIGHT_BLACK)
        );
    }
    if (config.previewLineStyle == NO_STYLE) {
        config.previewLineStyle = StyleManager::instance().add(
            AnsiStyle().underline().blend()
        );
    }
}

bool readConfig(int argc, const char **argv) {
    ConfigJsonReader configJsonReader;
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
