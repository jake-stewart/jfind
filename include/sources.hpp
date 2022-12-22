#ifndef SOURCES_HPP
#define SOURCES_HPP

#include "source.hpp"
#include <vector>
#include <fstream>

class Sources
{
private:
    int m_max_text_w;
    std::vector<Source> m_sources;
    void readWithHints(FILE *fp);
    void readWithoutHints(FILE *fp);

public:
    void read(FILE *fp, bool hints);
    bool readFile(const char *fname, bool hints);
    Source get(int i);
    int size();
    void sort(const char *query, bool skip_empty);
    int getMaxWidth();
};


#endif
