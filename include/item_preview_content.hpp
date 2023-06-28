#ifndef ITEM_PREVIEW_CONTENT_HPP
#define ITEM_PREVIEW_CONTENT_HPP

#include <string>
#include <vector>
#include "ansi_string.hpp"

struct ItemPreviewContent {
    int lineNumber;
    std::vector<AnsiString> lines;
};

#endif
