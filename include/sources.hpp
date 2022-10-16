#ifndef SOURCES_HPP
#define SOURCES_HPP

#include "source.hpp"
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Sources
{
private:
    std::vector<Source> m_sources;
    bool                m_has_heuristic   = false;
    int                 m_last_query_size = 0;

    void    resetHeuristics();

public:
    void    sort(std::string query);
    size_t  size();
    Source& get(int idx);
    int     getLargestSourceLength();
    bool    hasHeuristic();
    void    push_back(Source& source);
    void    read(std::istream& stream, bool hints);
    bool    readFile(fs::path path, bool hints);
};

#endif
