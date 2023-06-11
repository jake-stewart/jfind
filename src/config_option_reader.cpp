#include "../include/config_option_reader.hpp"
#include "../include/option_parser.hpp"
#include "../include/key.hpp"

bool ConfigOptionReader::read(int argc, const char **argv) {
    std::string historyFile;

    std::vector<Option *> options = {
        {new BooleanOption("help", &m_config.showHelp),
         new BooleanOption("hints", &m_config.showHints),
         new BooleanOption("select-hint", &m_config.selectHint),
         new BooleanOption("select-both", &m_config.selectBoth),
         new BooleanOption("accept-non-match", &m_config.acceptNonMatch),
         new StringOption("prompt", &m_config.prompt),
         new StringOption("query", &m_config.query),
         new StringOption("history", &historyFile),
         new StringOption("log", &m_config.logFile),
         new StringOption("command", &m_config.command),
         (new IntVectorOption("additional-keys", &m_config.additionalKeys))
             ->min(K_NULL)
             ->max(K_ERROR),
         (new BooleanOption("show-key", &m_config.showKey)),
         (new IntegerOption("history-limit", &m_config.historyLimit))->min(0)}};

    OptionParser optionParser(options);
    bool success = optionParser.parse(argc, argv);

    m_config.historyFile = historyFile;

    return success;
}
