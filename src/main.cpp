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

static Config& config = Config::instance();
static AnsiWrapper& ansi = AnsiWrapper::instance();
static EventDispatch& eventDispatch = EventDispatch::instance();
static Logger logger = Logger("main");

static InputReader inputReader;
static StyleManager styleManager(stderr);
static HistoryManager *historyManager = nullptr;
static ItemCache itemCache;
static ItemList itemList(stderr, &styleManager, &itemCache);
static Utf8LineEditor editor(stderr);
static UserInterface userInterface(stderr, &styleManager, &itemList, &editor);

void printResult(Key key, Item *selected, const char *input) {
    if (config.showKey && (selected || config.acceptNonMatch)) {
        printf("%d\n", key);
    }
    if (selected) {
        if (!config.selectHint || config.selectBoth) {
            printf("%s\n", selected->text);
        }
        if (config.selectHint || config.selectBoth) {
            printf("%s\n", selected->text + strlen(selected->text) + 1);
        }
    }
    else if (config.acceptNonMatch) {
        printf("%s\n", input);
    }
}

void createStyles(StyleManager *styleManager) {
    if (config.itemStyle == NO_STYLE) {
        config.itemStyle = styleManager->add(AnsiStyle().fg(BLUE));
    }
    if (config.hintStyle == NO_STYLE) {
        config.hintStyle = styleManager->add(AnsiStyle().fg(BRIGHT_BLACK));
    }
    if (config.activeItemStyle == NO_STYLE) {
        config.activeItemStyle = styleManager->add(AnsiStyle().fg(WHITE));
    }
    if (config.activeHintStyle == NO_STYLE) {
        config.activeHintStyle = styleManager->add(AnsiStyle().fg(WHITE));
    }
}

bool readConfig(StyleManager *styleManager, int argc, const char **argv) {
    ConfigJsonReader configJsonReader(styleManager);
    ConfigOptionReader configOptionReader;

    std::ifstream ifs(expandUserPath("~/.config/jfind/config.json"));
    if (ifs.is_open()) {
        if (!configJsonReader.read(ifs)) {
            fprintf(stderr, "Error in config.json on line %d: %s\n",
                configJsonReader.getError().line,
                configJsonReader.getError().message.c_str()
            );
            return false;
        }
    }

    if (!configOptionReader.read(argc, argv)) {
        return false;
    }

    return true;
}

void emitResizeEvent() {
    winsize ws;
    if (ioctl(STDERR_FILENO, TIOCGWINSZ, &ws) == -1) {
        ansi.restoreTerm();
        fprintf(stderr, "failed to query terminal size\n");
        exit(1);
    }
    eventDispatch.dispatch(
            std::make_shared<ResizeEvent>(ws.ws_col, ws.ws_row));
}

void signalHandler(int sig) {
    switch (sig) {
        case SIGTERM:
            ansi.restoreTerm();
            printResult(
                userInterface.getSelectedKey(), userInterface.getSelected(),
                editor.getText().c_str()
            );
            logger.log("done");
            Logger::close();
            exit(0);
            break;
        case SIGINT:
            eventDispatch.dispatch(std::make_shared<KeyEvent>(K_CTRL_C));
            break;
        case SIGWINCH:
            logger.log("received SIGWINCH");
            emitResizeEvent();
            break;
        default:
            logger.log("received unknown signal %d", sig);
            break;
    }
}

void displayHelp(const char *name) {
    printf("usage: %s [options]\n", name);
    printf("\n");
    printf("OPTIONS:\n");
    printf("    --help                        Display this dialog\n");
    printf("    --hints                       Read hints from stdin (every second line)\n");
    printf("    --select-hint                 Print the hint to stdout\n");
    printf("    --select-both                 Print both the item and hint to stdout\n");
    printf("    --accept-non-match            Accept the user's query if nothing matches\n");
    printf("    --matcher=MATCHER             Override matching algorithm to MATCHER (fuzzy or regex)\n");
    printf("    --case-mode=SENSITIVITY       Override case sensitivity to SENSITIVITY (sensitive, insensitive, or smart)\n");
    printf("    --history=FILE                Read and write match history to FILE\n");
    printf("    --history-limit=INT           Number of items to store in the history file\n");
    printf("    --prompt=PROMPT               Set the query prompt to PROMPT\n");
    printf("    --query=QUERY                 Set the starting query to QUERY\n");
    printf("    --command                     Use COMMAND to filter items. See the COMMAND section for details\n");
    printf("    --additional-keys=KEY_LIST    Accept the item at the cursor if a key in KEY_LIST is pressed.\n");
    printf("                                  KEY_LIST is provided as a comma separated list of ascii codes.\n");
    printf("    --show-key                    Print the keycode which was used to select an item.\n");
    printf("                                  Unless provided in --additional-keys, mouse and enter will both return 0\n");
    printf("\n");
    printf("CONFIG (~/.config/jfind/config.json):\n");
    printf("    selector: STRING              The selector of an unselected item\n");
    printf("    active_selector: STRING       The selector of the selected item\n");
    printf("    prompt: STRING                The default prompt\n");
    printf("    prompt_gap: INT               The distance between the prompt and query\n");
    printf("    history_limit: INT            Default number of items to store in the history file\n");
    printf("    min_hint_spacing: INT         Minimum gap between an item and its hint\n");
    printf("    min_hint_width: INT           Minimum width a hint should be before it is shown\n");
    printf("    max_hint_width: INT           Maximum width a hint can grow to\n");
    printf("    show_spinner: BOOL            Show a spinner animation at the bottom right when loading\n");
    printf("    style: STYLE OBJECT           Custom styles. See STYLES for keys, and STYLE OBJECT for values\n");
    printf("    matcher: MATCHER              Default matching algorithm (fuzzy or regex)\n");
    printf("    case_mode: SENSITIVITY        Default case sensitivity (sensitive, insensitive, or smart)\n");
    printf("\n");
    printf("STYLES:\n");
    printf("    item                          An unselected item\n");
    printf("    active_item                   A selected item\n");
    printf("    hint                          The hint of an unselected item\n");
    printf("    active_hint                   The hint of a selected item\n");
    printf("    selector                      The selector of an unselected item\n");
    printf("    active_selector               The selector of a selected item\n");
    printf("    active_row                    The gap between a selected item and its hint\n");
    printf("    row                           The gap between an unselected item and its hint\n");
    printf("    search_prompt                 The query prompt\n");
    printf("    search                        The query that the user enters\n");
    printf("    search_row                    Everywhere else on the search row\n");
    printf("    background                    Everywhere else on the screen\n");
    printf("\n");
    printf("STYLE OBJECT:\n");
    printf("    fg: STRING                    Foreground color as one of COLOR NAMES or a hex string\n");
    printf("    bg: STRING                    Background color as one of COLOR NAMES or a hex string\n");
    printf("    attr: LIST                    A list of ATTRIBUTES\n");
    printf("\n");
    printf("COLOR NAMES:\n");
    printf("    red, green, blue, cyan, yellow, magenta, black, white\n");
    printf("    A color name may be prefixed with 'bright_' to use the bright variant\n");
    printf("\n");
    printf("ATTRIBUTES:\n");
    printf("    bold, italic, blink, reverse,\n");
    printf("    underline, curly_underline, double_underline, dotted_underline, dashed_underline\n");
    printf("\n");
    printf("COMMAND:\n");
    printf("    You can use the --command flag to run spawn a process whenever the query changes.\n");
    printf("    The stdout of this process will be displayed as items in jfind.\n");
    printf("    For an interactive example, run 'jfind --command=\"seq {}\"'.\n");
    printf("\n");
    printf("Input should be piped into jfind for it to be useful. You can try the following example:\n");
    printf("    seq 100 | %s\n", name);
}

int main(int argc, const char **argv) {

    ItemSorter *itemSorter;
    ItemMatcher *matcher;
    FileItemReader *fileItemReader;
    ProcessItemReader *processItemReader;

    if (!readConfig(&styleManager, argc, argv)) {
        return 1;
    }

    if (config.logFile.size()) {
        Logger::open(config.logFile.c_str());
    }

    if ((!config.command.size() && isatty(STDIN_FILENO)) || config.showHelp) {
        displayHelp(argv[0]);
        return 0;
    }

    createStyles(&styleManager);

    if (!config.historyFile.empty()) {
        historyManager = new HistoryManager(config.historyFile);
        historyManager->setHistoryLimit(config.historyLimit);
        historyManager->readHistory();
    }

    editor.input(config.query);

    if (config.command.size()) {
        processItemReader = new ProcessItemReader(config.command, config.query);

        userInterface.setThreadsafeReading(true);

        itemCache.setItemsCallback(
            [processItemReader] (Item *buffer, int idx, int n) {
                return processItemReader->copyItems(buffer, idx, n);
            }
        );
        itemCache.setSizeCallback(
            [processItemReader] () {
                return processItemReader->size();
            }
        );
    }
    else {
        itemSorter = new ItemSorter(config.query);
        userInterface.setThreadsafeReading(false);
        switch (Config::instance().matcher) {
            case FUZZY_MATCHER:
                matcher = new ItemFuzzyMatcher();
                break;
            case REGEX_MATCHER:
                matcher = new ItemRegexMatcher();
                break;
            case EXACT_MATCHER:
                matcher = new ItemExactMatcher();
                break;
        }
        itemSorter->setMatcher(matcher);

        fileItemReader = new FileItemReader(stdin);

        itemCache.setItemsCallback(
            [itemSorter] (Item *buffer, int idx, int n) {
                return itemSorter->copyItems(buffer, idx, n);
            }
        );
        itemCache.setSizeCallback(
            [itemSorter] () {
                return itemSorter->size();
            }
        );
    }

    int fd = open("/dev/tty", O_RDONLY);
    ansi.setInputFileNo(fd);
    inputReader.setFileDescriptor(fd);

    // enable unicode
    setlocale(LC_ALL, "en_US.UTF-8");

    ansi.setOutputFile(stderr);

    signal(SIGWINCH, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    ansi.initTerm();
    ansi.enableMouse();
    ansi.setCursor(true);
    emitResizeEvent();

    std::vector<std::thread> threads;
    threads.push_back(std::thread(&UserInterface::start, &userInterface));
    if (config.command.size()) {
        close(STDIN_FILENO);
        threads.push_back(std::thread(&ProcessItemReader::start, processItemReader));
    }
    else {
        threads.push_back(std::thread(&ItemSorter::start, itemSorter));
        threads.push_back(std::thread(&FileItemReader::start, fileItemReader));
    }
    threads.push_back(std::thread(&InputReader::start, &inputReader));
    for (std::thread &thread : threads) {
        thread.join();
    }

    Item *selected = userInterface.getSelected();
    if (selected && historyManager) {
        historyManager->writeHistory(selected);
        delete historyManager;
    }

    ansi.restoreTerm();

    printResult(userInterface.getSelectedKey(), selected,
            editor.getText().c_str());
    logger.log("done");
    Logger::close();

    if (config.command.size()) {
        delete processItemReader;
    }
    else {
        delete itemSorter;
        delete fileItemReader;
        delete matcher;
    }

    return 0;
}

