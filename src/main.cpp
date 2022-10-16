#include "../include/util.hpp"
#include "../include/sources.hpp"
#include "../include/config.hpp"
#include "../include/input_handler.hpp"
#include "../include/history_manager.hpp"
#include "../include/config_json_reader.hpp"
#include "../include/config_option_reader.hpp"
#include "../include/ansi_wrapper.hpp"

#include <iostream>
#include <fstream>

Config config;

int          width;
int          height;
Sources      sources;
int          n_visible_sources;
InputHandler input_handler;

void drawSources(int offset) {
    if (offset >= height) {
        offset = height - 1;
    }

    n_visible_sources = sources.size() > height - 1
        ? height - 1
        : sources.size();

    int col_width = 0;
    for (int i = 0; i < n_visible_sources; i++) {
        if (sources.hasHeuristic() &&
                sources.get(i).getHeuristic() == INT_MAX) {
            n_visible_sources = i;
            break;
        }

        std::string name = sources.get(i).getName();

        if (name.size() > width) {
            name = name.substr(0, width - 1) + "…";
        }
        if (name.size() > col_width) {
            col_width = name.size();
        }

        if (i == offset) {
            move(0, height - i - 2);
            printf("%s\x1b[K", config.selectedRowStyle.c_str());
            if (config.selector.size()) {
                printf("%s", config.selectorStyle.c_str());
                printf("%s", config.selector.c_str());
            }
            printf("%s", config.selectedSourceStyle.c_str());
        }
        else {
            move(config.selector.size(), height - i - 2);
            printf("%s\x1b[K", config.rowStyle.c_str());
            printf("%s", config.sourceStyle.c_str());
        }

        printf("%s", name.c_str());
    }

    int x = col_width + config.minInfoSpacing + config.selector.size();
    int w = width - x;
    if (w > config.maxInfoWidth) {
        x += w - config.maxInfoWidth;
        w = config.maxInfoWidth;
    }

    if (w >= config.minInfoWidth) {
        for (int i = 0; i < n_visible_sources; i++) {
            if (sources.hasHeuristic() &&
                    sources.get(i).getHeuristic() == INT_MAX) {
                break;
            }
            printf("%s", (i == offset)
                    ? config.selectedHintStyle.c_str()
                    : config.hintStyle.c_str());

            if (sources.get(i).getPath().size() > w) {
                const char *str = sources.get(i).getPath().c_str();
                int start_idx = sources.get(i).getPath().size() - w + 1;
                int idx = start_idx;
                while (str[idx] != '/') {
                   idx++;
                   if (idx == sources.get(i).getPath().size()) {
                       idx = start_idx;
                       break;
                   }
                }
                move(
                    width - sources.get(i).getPath().size() + idx - 1,
                    height - i - 2
                );
                printf("…");
                printf("%s", str + idx);
            }
            else {
                move(
                    width - sources.get(i).getPath().size(),
                    height - i - 2
                );
                printf("%s", sources.get(i).getPath().c_str());
            }
        }
    }
}

void drawQuery(std::string query) {
    move(0, height - 1);
    printf("%s\x1b[K", config.searchRowStyle.c_str());

    printf("%s", config.searchPromptStyle.c_str());
    printf("%s", config.prompt.c_str());
    move(config.prompt.size() + 1, height - 1);
    printf("%s", config.searchStyle.c_str());

    if (query.size() + 3 >= width) {
        int idx = query.size() - width + 3;
        printf("%s", query.c_str() + idx);
    }
    else {
        printf("%s", query.c_str());
    }

    int x = input_handler.getCursorPos();
    move(config.prompt.size() + x + 1, height - 1);
}

void drawScreen() {
    printf("%s", config.backgroundStyle.c_str());
    clear_term();
    drawSources(input_handler.getOffset());
    drawQuery(input_handler.getQuery());
}

void onResize(int new_width, int new_height) {
    width = new_width;
    height = new_height;
    drawScreen();
}

int main(int argc, const char **argv) {
    ConfigJsonReader configJsonReader;
    ConfigOptionReader configOptionReader;

    if (!configJsonReader.read("~/.config/jfind/config.json", &config)) {
        printf("Error in config.json on line %d: %s\n",
            configJsonReader.getErrorLine(),
            configJsonReader.getError().c_str()
        );
        return 1;
    }

    if (!configOptionReader.read(argc, argv, &config)) {
        return 1;
    }

    input_handler.setQuery(config.query);
    HistoryManager::historyCount = config.historyCount;

    if (config.sourceFile.empty()) {
        sources.read(std::cin, config.showHints);
    }
    else {
        if (!sources.readFile(config.sourceFile, config.showHints)) {
            return 1;
        }
        std::cin.clear();
    }

    HistoryManager *historyManager = nullptr;
    if (!config.historyFile.empty()) {
        historyManager = new HistoryManager(config.historyFile);
        historyManager->readHistory(sources);
    }

    // reopen stdin since reaching EOF breaks stdin
    freopen("/dev/tty", "rw", stdin);

    // enable unicode
    setlocale(LC_ALL, "");

    register_resize_callback(onResize);
    useStderr(true);
    init_term();

    while (input_handler.isActive()) {
        sources.sort(input_handler.getQuery());
        input_handler.setOffsetRange(n_visible_sources);

        drawScreen();

        input_handler.handle();
    }

    restore_term();

    if (input_handler.isSelected() && n_visible_sources) {
        Source& source = sources.get(input_handler.getOffset());
        if (historyManager) {
            historyManager->writeHistory(source, sources);
        }
        if (!config.selectHint || config.selectBoth) {
            printf("%s\n", source.getName().c_str());
        }
        if (config.selectHint || config.selectBoth) {
            printf("%s\n", source.getPath().c_str());
        }
    }
    else if (config.acceptNonMatch) {
        for (int i = 0; i < config.selectBoth + 1; i++) {
            printf("%s\n", input_handler.getQuery().c_str());
        }
    }

    return 0;
}

