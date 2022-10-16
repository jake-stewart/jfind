#ifndef SOURCE_HPP
#define SOURCE_HPP

#include <string>
#include <vector>

class Source
{
private:
    std::string               m_name;
    std::string               m_lower_name;
    std::vector<std::string>  m_words;
    std::string               m_path;
    int                       m_heuristic;
    int                       m_history;

public:
    Source();

    void         calcHeuristic(std::string& query,
                               std::vector<std::string>& words);

    int          getHeuristic();
    std::string& getPath();
    std::string& getName();
    int          getHistory();

    void        setHeuristic(int heuristic);
    void        setName(std::string name);
    void        setPath(std::string src_path);
    void        setHistory(int history);
};

#endif
