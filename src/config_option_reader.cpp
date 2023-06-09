#include "../include/config_option_reader.hpp"
#include "../include/option_parser.hpp"

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
         new EnumOption<MatcherType>(
             "matcher", &m_config.matcher,
             std::map<std::string, MatcherType>{
                 {"fuzzy", FUZZY_MATCHER},
                 {"regex", REGEX_MATCHER}}
         ),
         (new IntegerOption("history-limit", &m_config.historyLimit))->min(0)}};

    OptionParser optionParser(options);
    bool success = optionParser.parse(argc, argv);

    m_config.historyFile = historyFile;

    return success;
}
