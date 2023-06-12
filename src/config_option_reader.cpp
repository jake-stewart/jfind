#include "../include/config_option_reader.hpp"
#include "../include/option_parser.hpp"
#include "../include/key.hpp"

bool ConfigOptionReader::read(int argc, const char **argv) {
    Config &config = Config::instance();
    std::string historyFile;

    std::vector<Option *> options = {
        {new BooleanOption("help", &config.showHelp),
         new BooleanOption("hints", &config.showHints),
         new BooleanOption("select-hint", &config.selectHint),
         new BooleanOption("select-both", &config.selectBoth),
         new BooleanOption("accept-non-match", &config.acceptNonMatch),
         new StringOption("prompt", &config.prompt),
         new StringOption("query", &config.query),
         new StringOption("history", &historyFile),
         new StringOption("log", &config.logFile),
         new StringOption("command", &config.command),
         (new IntVectorOption("additional-keys", &config.additionalKeys))
             ->min(K_NULL)
             ->max(K_ERROR),
         (new BooleanOption("show-key", &config.showKey)),
         (new IntegerOption("history-limit", &config.historyLimit))->min(0)}};

    OptionParser optionParser(options);
    bool success = optionParser.parse(argc, argv);

    config.historyFile = historyFile;

    return success;
}
