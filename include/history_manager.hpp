#ifndef SOURCE_PARSER
#define SOURCE_PARSER

#include "source.hpp"
#include "sources.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

class HistoryManager
{
    private:
        std::map<std::string, int> m_history_lookup;
        fs::path m_file;
        bool createHistoryLookup();

    public:
        static int historyCount;

        HistoryManager(fs::path file);
        bool readHistory(Sources& sources);
        bool writeHistory(Source& selected, Sources& sources);
};

#endif
