#include "../include/util.hpp"
#include "../include/sources.hpp"
#include "../include/config.hpp"
#include "../include/history_manager.hpp"
#include "../include/config_json_reader.hpp"
#include "../include/config_option_reader.hpp"
#include "../include/ansi_wrapper.hpp"
#include "../include/key.hpp"
#include "../include/input_reader.hpp"
#include "../include/utf8_line_editor.hpp"
#include "../include/style_manager.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <climits>
#include <thread>
#include <condition_variable>
#include <mutex>

std::mutex mut;
std::condition_variable cv;
bool sorter_thread_active = false;
bool deleted = false;

namespace chrono = std::chrono;

Config config;
StyleManager style_manager;

int          width;
int          height;
Sources      sources;
int          n_visible_sources;
int          n_active_sources;

Utf8LineEditor editor;
InputReader reader;

int offset = 0;
int cursor = 0;
bool active = true;
bool selected = false;

int hint_w;

void drawName(int i) {
    std::string name = std::string(sources.get(i).text);

    if (name.size() > width) {
        name = name.substr(0, width - 1) + "…";
    }

    move(0, height - i - 2 + offset);
    if (i == cursor) {
        style_manager.set(config.activeRowStyle);
        clear_til_eol();
        if (config.activeSelector.size()) {
            style_manager.set(config.activeSelectorStyle);
            printf("%s", config.activeSelector.c_str());
        }
        style_manager.set(config.activeSourceStyle);
    }
    else {
        style_manager.set(config.rowStyle);
        clear_til_eol();
        if (config.selector.size()) {
            style_manager.set(config.selectorStyle);
            printf("%s", config.selector.c_str());
        }
        style_manager.set(config.sourceStyle);
    }

    printf("%s", name.c_str());
}

void drawHint(int i) {
    char *text = sources.get(i).text;
    std::string hint = std::string(text + strlen(text) + 1);

    style_manager.set(i == cursor ? config.activeHintStyle : config.hintStyle);

    if (hint.size() > hint_w) {
        const char *str = hint.data();
        int start_idx = hint.size() - hint_w + 1;
        int idx = start_idx;
        while (str[idx] != '/') {
           idx++;
           if (idx == hint.size()) {
               idx = start_idx;
               break;
           }
        }
        move(
            width - hint.size() + idx - 1,
            height - i - 2 + offset
        );
        printf("…");
        printf("%s", str + idx);
    }
    else {
        move(
            width - hint.size(),
            height - i - 2 + offset
        );
        printf("%s", hint.data());
    }
}

void drawSources() {
    for (int i = offset; i < offset + n_visible_sources; i++) {
        drawName(i);
    }

    if (hint_w >= config.minInfoWidth) {
        for (int i = offset; i < n_visible_sources + offset; i++) {
            drawHint(i);
        }
    }
    if (height - n_visible_sources - 2 > 0) {
        move(width - 1, height - n_visible_sources - 2);
        style_manager.set(config.backgroundStyle);
        clear_til_sof();
    }
}

void drawPrompt() {
    move(0, height - 1);
    style_manager.set(config.searchRowStyle);
    clear_til_eol();
    style_manager.set(config.searchPromptStyle);
    printf("%s", config.prompt.c_str());
    move(config.prompt.size() + config.promptGap, height - 1);
    style_manager.set(config.searchStyle);
}

void drawQuery() {
    move(config.prompt.size() + config.promptGap, height - 1);
    style_manager.set(config.searchRowStyle);
    clear_til_eol();
    style_manager.set(config.searchStyle);
    editor.print();
}

void focusEditor() {
    move(
        config.prompt.size() + config.promptGap + editor.getCursorCol(),
        height - 1
    );
}

void onResize(int new_width, int new_height) {
    width = new_width;
    height = new_height;

    n_visible_sources = n_active_sources > height - 1
        ? height - 1
        : n_active_sources;

    if (height - 1 + offset > n_active_sources) {
        offset = n_active_sources - (height - 1);
        if (offset < 0) {
            offset = 0;
        }
    }
    else if (cursor - offset >= n_visible_sources) {
        offset = cursor - n_visible_sources + 1;
    }

    int col_width = sources.getMaxWidth();
    if (col_width > width) {
        col_width = width;
    }

    int x = col_width + config.minInfoSpacing
        + (config.selector.size() > config.activeSelector.size()
                ? config.selector.size()
                : config.activeSelector.size());
    hint_w = width - x;

    if (hint_w > config.maxInfoWidth) {
        x += hint_w - config.maxInfoWidth;
        hint_w = config.maxInfoWidth;
    }

    drawSources();
    drawPrompt();
    editor.setWidth(width - 1);
    drawQuery();
    focusEditor();
    // refresh();
}

void moveCursorDown() {
    if (cursor <= 0) {
        return;
    }
    cursor -= 1;
    if (cursor - offset < 0) {
        offset -= 1;

        move(0, height - 1);
        move_down_or_scroll();

        drawPrompt();
        drawQuery();
    }
    drawName(cursor + 1);
    drawName(cursor);
    if (hint_w > config.minInfoWidth) {
        drawHint(cursor + 1);
        drawHint(cursor);
    }
}

void moveCursorUp() {
    if (cursor >= sources.size() - 1) {
        return;
    }
    if (sources.get(cursor + 1).heuristic == INT_MAX) {
        return;
    }

    cursor += 1;
    if (cursor - offset >= n_visible_sources) {
        offset += 1;
        move_home();
        move_up_or_scroll();
        drawPrompt();
        drawQuery();
    }
    drawName(cursor - 1);
    drawName(cursor);
    if (hint_w > config.minInfoWidth) {
        drawHint(cursor - 1);
        drawHint(cursor);
    }
}

void scrollUp() {
    if (offset + height > n_active_sources) {
        return;
    }
    move_home();
    move_up_or_scroll();
    offset += 1;
    if (cursor - offset < 0) {
        cursor += 1;
        drawName(offset);
        if (hint_w > config.minInfoWidth) {
            drawHint(offset);
        }
    }

    drawName(offset + height - 2);
    if (hint_w > config.minInfoWidth) {
        drawHint(offset + height - 2);
    }

    drawPrompt();
    drawQuery();
}

void scrollDown() {
    if (offset <= 0) {
        return;
    }
    move(0, height - 1);
    move_down_or_scroll();
    offset -= 1;
    if (cursor - offset >= height - 1) {
        cursor -= 1;
        drawName(offset + height - 2);
        if (hint_w > config.minInfoWidth) {
            drawHint(offset + height - 2);
        }
    }

    drawName(offset);
    if (hint_w > config.minInfoWidth) {
        drawHint(offset);
    }

    drawPrompt();
    drawQuery();
}

chrono::time_point<chrono::system_clock> start;

void handleClick(int x, int y) {
    bool can_double_click = true;
    int new_cursor = offset + (height - 1 - y);
    if (new_cursor < 0) {
        return;
    }
    if (new_cursor >= n_active_sources) {
        new_cursor = n_active_sources - 1;
        can_double_click = false;
    }

    if (cursor == new_cursor) {
        chrono::time_point<chrono::system_clock> end;
        end = chrono::system_clock::now();

        chrono::milliseconds delta;
        delta = chrono::duration_cast<chrono::milliseconds>(end - start);
        if (can_double_click && delta.count() < 250) {
            active = false;
            selected = true;
        }
    }
    else {
        int old_cursor = cursor;
        cursor = new_cursor;
        drawName(old_cursor);
        drawName(cursor);
        if (hint_w > config.minInfoWidth) {
            drawHint(old_cursor);
            drawHint(cursor);
        }
    }
    start = chrono::system_clock::now();
}

void handleMouse(MouseEvent event) {
    switch (event.button) {
        case MB_SCROLL_UP:
            scrollUp();
            break;
        case MB_SCROLL_DOWN:
            scrollDown();
            break;
        case MB_LEFT:
            if (event.pressed && !event.dragged) {
                if (event.y == height) {
                    editor.handleClick(
                        event.x - config.prompt.size() - config.promptGap
                    );
                }
                else {
                    handleClick(event.x, event.y);
                }
            }
            break;
        default:
            break;
    }
}

void handle_input() {
    Key key;
    do {
        reader.getKey(&key);

        switch (key) {
            case K_ESCAPE:
            case K_CTRL_C:
                active = false;
                break;

            case 32 ... 126:
                editor.input(key);
                break;

            case K_CTRL_A:
                editor.input(editor.getText());
                break;

            case K_CTRL_H:
            case K_BACKSPACE:
                editor.backspace();
                break;

            case K_DELETE:
                editor.del();
                break;

            case K_CTRL_U:
                editor.clear();
                break;

            case K_UP:
            case K_CTRL_K:
                moveCursorUp();
                break;

            case K_DOWN:
            case K_CTRL_J:
                moveCursorDown();
                break;

            case K_LEFT:
                editor.moveCursorLeft();
                break;

            case K_RIGHT:
                editor.moveCursorRight();
                break;

            case K_ENTER:
                selected = true;
                active = false;
                break;

            case K_UTF8:
                editor.input(reader.getWideChar());
                break;

            case K_MOUSE:
                handleMouse(reader.getMouseEvent());
                break;

            default:
                break;
        }
    }
    while (reader.hasKey());
}

void sortSources(std::string& query) {
    sources.sort(query.c_str(), false);

    n_active_sources = sources.size();

    for (int i = 0; i < sources.size(); i++) {
        if (sources.get(i).heuristic == INT_MAX) {
            n_active_sources = i;
            break;
        }
        // if (sources.hasHeuristic() &&
        //         sources.get(i).getHeuristic() == INT_MAX) {
        //     n_active_sources = i;
        //     break;
        // }
    }
    n_visible_sources = n_active_sources > height - 1
        ? height - 1
        : n_active_sources;
}

void sorter_thread_func() {
    std::string query = "";
    bool skip_empty;
    while (sorter_thread_active) {
        while (true) {
            if (!sorter_thread_active) {
                return;
            }

            std::unique_lock lock(mut);
            if (editor.getText() != query) {
                query = editor.getText();
                skip_empty = !deleted;
                deleted = false;
                break;
            }
            cv.wait(lock);
        }

        sources.sort(query.c_str(), skip_empty);
        {
            std::unique_lock lock(mut);
            offset = 0;
            cursor = 0;

            n_active_sources = sources.size();

            for (int i = 0; i < sources.size(); i++) {
                if (sources.get(i).heuristic == INT_MAX) {
                    n_active_sources = i;
                    break;
                }
                // if (sources.hasHeuristic() &&
                //         sources.get(i).getHeuristic() == INT_MAX) {
                //     n_active_sources = i;
                //     break;
                // }
            }
            n_visible_sources = n_active_sources > height - 1
                ? height - 1
                : n_active_sources;

            drawSources();
            focusEditor();
            // refresh();
        }
    }
}

int main(int argc, const char **argv) {
    ConfigJsonReader configJsonReader(&config, &style_manager);
    ConfigOptionReader configOptionReader(&config);

    if (!configJsonReader.read("~/.config/jfind/config.json")) {
        printf("Error in config.json on line %d: %s\n",
            configJsonReader.getErrorLine(),
            configJsonReader.getError().c_str()
        );
        return 1;
    }

    if (config.sourceStyle == NO_STYLE) {
        config.sourceStyle = style_manager.add(AnsiStyle().fg(BLUE));
    }
    if (config.hintStyle == NO_STYLE) {
        config.hintStyle = style_manager.add(AnsiStyle().fg(BRIGHT_BLACK));
    }
    if (config.activeSourceStyle == NO_STYLE) {
        config.activeSourceStyle = style_manager.add(AnsiStyle().fg(WHITE));
    }
    if (config.activeHintStyle == NO_STYLE) {
        config.activeHintStyle = style_manager.add(AnsiStyle().fg(WHITE));
    }

    if (!configOptionReader.read(argc, argv)) {
        return 1;
    }

    HistoryManager::historyCount = config.historyCount;

    if (config.sourceFile.empty()) {
        sources.read(stdin, config.showHints);
    }
    else {
        if (!sources.readFile(config.sourceFile.c_str(), config.showHints)) {
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
    setlocale(LC_ALL, "en_US.UTF-8");

    // std::string query = "ninini";
    // sortSources(query);
    // printf("%d intmax=%d\n", sources.get(0).heuristic, sources.get(0).heuristic == INT_MAX);

    // return 0;

    sortSources(config.query);

    register_resize_callback(onResize);
    useStderr(true);
    init_term();
    enable_mouse();
    set_cursor(true);

    editor.input(config.query);

    drawQuery();

    sorter_thread_active = true;
    std::thread sorter_thread = std::thread(sorter_thread_func);

    while (active) {
        {
            std::unique_lock lock(mut);
            focusEditor();
            // refresh();
        }
        int s = editor.getText().size();
        handle_input();
        {
            std::unique_lock lock(mut);
            if (editor.isModified()) {
                if (editor.getText().size() < s) {
                    deleted = true;
                }
                cv.notify_one();
            }
            if (editor.requiresRedraw()) {
                drawQuery();
            }
        }
    }

    {
        std::unique_lock lock(mut);
        sorter_thread_active = false;
        cv.notify_one();
    }
    sorter_thread.join();

    restore_term();

    if (selected && n_visible_sources) {
        Source source = sources.get(cursor);
        if (historyManager) {
            historyManager->writeHistory(source, sources);
        }
        if (!config.selectHint || config.selectBoth) {
            printf("%s\n", source.text);
        }
        if (config.selectHint || config.selectBoth) {
            printf("%s\n", source.text + strlen(source.text) + 1);
        }
    }
    else if (config.acceptNonMatch) {
        for (int i = 0; i < config.selectBoth + 1; i++) {
            printf("%s\n", editor.getText().c_str());
        }
    }

    return 0;
}

