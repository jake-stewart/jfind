#include "../include/config_option_reader.hpp"
#include "../include/key.hpp"
#include "../include/option_parser.hpp"

bool ConfigOptionReader::read(int argc, const char **argv) {
    Config &config = Config::instance();
    std::string historyFile;

    std::vector<Option *> options = {
        {new BooleanOption("help", &config.showHelp),
         new BooleanOption("hints", &config.showHints),
         new BooleanOption("select-hint", &config.selectHint),
         new BooleanOption("accept-non-match", &config.acceptNonMatch),
         new StringOption("prompt", &config.prompt),
         new StringOption("query", &config.query),
         new StringOption("history", &historyFile),
         new StringOption("log", &config.logFile),
         new StringOption("command", &config.command),
         new StringOption("preview", &config.preview),
         new StringOption("preview-line", &config.previewLine),
         new BooleanOption("external-border", &config.externalBorder),
         new EnumOption(
             "preview-position", &config.previewPlacement, {
                 {"top", Placement::Top},
                 {"bottom", Placement::Bottom},
                 {"left", Placement::Left},
                 {"right", Placement::Right}
             }
         ),
         (new FloatOption(
             "percent-preview", &config.percentPreview
         ))->min(0)->max(1),
         new EnumOption(
             "query-position", &config.queryPlacement, {
                 {"top", VerticalPlacement::Top},
                 {"bottom", VerticalPlacement::Bottom},
             }
         ),
         (new IntVectorOption("additional-keys", &config.additionalKeys))
             ->min(K_NULL)
             ->max(K_ERROR),
         (new BooleanOption("show-key", &config.showKey)),
         (new IntegerOption("history-limit", &config.historyLimit))->min(0),
         new EnumOption(
             "matcher", &config.matcher, {
                 {"fuzzy", FUZZY_MATCHER},
                 {"regex", REGEX_MATCHER},
                 {"exact", EXACT_MATCHER}
             }
         ),
         new EnumOption(
             "case-mode", &config.caseSensitivity, {
                 {"sensitive", CASE_SENSITIVE},
                 {"insensitive", CASE_INSENSITIVE},
                 {"smart", SMART_CASE}
             }
         )}};

    OptionParser optionParser(options);
    bool success = optionParser.parse(argc, argv);

    for (const Option *option : options) {
        delete option;
    }

    config.historyFile = historyFile;

    return success;
}
