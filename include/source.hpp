#ifndef SOURCE_HPP
#define SOURCE_HPP

struct Source
{
public:
    char *text;
    int   heuristic;
    int   history;

public:
    Source(char *text) {
        this->text = text;
    }
};

#endif
