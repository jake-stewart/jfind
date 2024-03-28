#include "../include/ansi_wrapper.hpp"
#include "../include/config.hpp"
#include "../include/config_json_reader.hpp"
#include "../include/config_option_reader.hpp"
#include "../include/event_dispatch.hpp"
#include "../include/file_item_reader.hpp"
#include "../include/history_manager.hpp"
#include "../include/item_cache.hpp"
#include "../include/item_exact_matcher.hpp"
#include "../include/item_fuzzy_matcher.hpp"
#include "../include/item_list.hpp"
#include "../include/item_matcher.hpp"
#include "../include/item_regex_matcher.hpp"
#include "../include/item_sorter.hpp"
#include "../include/jfind_strategy.hpp"
#include "../include/logger.hpp"
#include "../include/process_item_reader.hpp"
#include "../include/style_manager.hpp"
#include "../include/user_interface.hpp"
#include "../include/utf8_line_editor.hpp"
#include "../include/util.hpp"
#include <climits>
#include <csignal>
#include <cstring>
#include <thread>

extern "C" {
#include <fcntl.h>
}

static Config &config = Config::instance();
static AnsiWrapper &ansi = AnsiWrapper::instance();
static EventDispatch &eventDispatch = EventDispatch::instance();

static InputReader inputReader;
static ItemCache itemCache;
static ItemList itemList(&itemCache);
static Utf8LineEditor editor(stderr);
static UserInterface userInterface(&itemList, &editor);
static JfindStrategy *jfindStrategy = nullptr;

void printResult(Key key, Item *selected, const char *input) {
    if (config.showKey && (selected || config.acceptNonMatch)) {
        printf("%d\n", key);
    }
    if (config.showQuery) {
        printf("%s\n", editor.getText().c_str());
    }
    if (selected) {
        const char *result = selected->text;
        if (config.selectHint) {
            result += strlen(result) + 1;
        }
        printf("%s\n", result);
    }
    else if (config.acceptNonMatch) {
        printf("%s\n", input);
    }
}

winsize getWinsize() {
    winsize ws;
    if (ioctl(STDERR_FILENO, TIOCGWINSZ, &ws) == -1) {
        ansi.restoreTerm();
        fprintf(stderr, "failed to query terminal size\n");
        exit(1);
    }
    return ws;
}

void finish() {
    ansi.restoreTerm();
    Item *selected = userInterface.getSelected();
    if (selected) {
        HistoryManager::instance().writeHistory(editor.getText());
    }
    if (config.selectAll && jfindStrategy) {
        if (config.showKey) {
            printf("%d\n", userInterface.getSelectedKey());
        }
        if (selected) {
            const char *result = selected->text;
            if (config.selectHint) {
                result += strlen(result) + 1;
            }
            printf("%s\n", result);
            for (const Item &item : jfindStrategy->getItems()) {
                if (item.heuristic == BAD_HEURISTIC || item.index == selected->index) {
                    continue;
                }
                const char *result = item.text;
                if (config.selectHint) {
                    result += strlen(result) + 1;
                }
                printf("%s\n", result);
            }
        }
    }
    else {
        printResult(
            userInterface.getSelectedKey(), selected, editor.getText().c_str()
        );
    }
    LOG("done");
    Logger::instance().close();
    exit(0);
}

void signalHandler(int sig) {
    switch (sig) {
        case SIGUSR1:
        case SIGTERM:
            finish();
            break;
        case SIGINT:
            eventDispatch.dispatch(std::make_shared<KeyEvent>(K_CTRL_C));
            break;
        case SIGWINCH: {
            LOG("received SIGWINCH");
            winsize ws = getWinsize();
            eventDispatch.dispatch(
                std::make_shared<ResizeEvent>(ws.ws_col, ws.ws_row)
            );
            break;
        }
        default:
            LOG("received unknown signal %d", sig);
            break;
    }
}

int main(int argc, const char **argv) {
    if (!readConfig(argc, argv)) {
        return 1;
    }

    if (config.logFile.size()) {
        Logger::instance().open(config.logFile.c_str());
    }

    if ((!config.command.size() && isatty(STDIN_FILENO)) || config.showHelp) {
        displayHelp(argv[0]);
        return 0;
    }

    StyleManager::instance().setOutputFile(stderr);
    createStyles();

    itemList.showCursor(true);

    editor.input(config.query);

    int fd = open("/dev/tty", O_RDONLY);
    ansi.setInputFileNo(fd);
    inputReader.setFileDescriptor(fd);

    // enable unicode
    setlocale(LC_ALL, "en_US.UTF-8");

    ansi.setOutputFile(stderr);

    signal(SIGWINCH, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    ansi.initTerm();
    ansi.enableMouse();
    ansi.setCursor(true);
    winsize ws = getWinsize();
    userInterface.onEvent(std::make_shared<ResizeEvent>(ws.ws_col, ws.ws_row));

    jfindStrategy = config.command.size()
        ? (JfindStrategy *)new InteractiveCommandStrategy(itemCache)
        : (JfindStrategy *)new FuzzyFindStrategy(itemCache);

    ItemPreviewReader *previewReader;
    std::thread *previewReaderThread;
    if (Config::instance().preview.size()) {
        previewReader = new ItemPreviewReader();
        previewReaderThread = new std::thread(
            &ItemPreviewReader::start, previewReader
        );
    }

    userInterface.setThreadsafeReading(jfindStrategy->isThreadsafeReading());
    jfindStrategy->start();
    new std::thread(&InputReader::start, &inputReader);

    userInterface.start();

    LOG("edited unexpectedly");
    Logger::instance().close();
    ansi.restoreTerm();
    return 1;
}
