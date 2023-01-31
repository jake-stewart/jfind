#include "../include/item_sorter.hpp"
#include "../include/item_cache.hpp"
#include "../include/config.hpp"
#include "../include/util.hpp"
#include "../include/history_manager.hpp"
#include "../include/config_json_reader.hpp"
#include "../include/config_option_reader.hpp"
#include "../include/ansi_wrapper.hpp"
#include "../include/utf8_line_editor.hpp"
#include "../include/style_manager.hpp"
#include "../include/user_interface.hpp"
#include "../include/thread_coordinator.hpp"

#include <climits>
#include <fcntl.h>

Config config;
UserInterface userInterface;
HistoryManager *historyManager = nullptr;
ItemSorter sorter;

void printResult(Item *selected) {
    if (selected) {
        if (historyManager) {
            historyManager->writeHistory(selected);
        }
        if (!config.selectHint || config.selectBoth) {
            printf("%s\n", selected->text);
        }
        if (config.selectHint || config.selectBoth) {
            printf("%s\n", selected->text + strlen(selected->text) + 1);
        }
    }
    else if (config.acceptNonMatch) {
        printf("%s\n", userInterface.getEditor()->getText().c_str());
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
    ConfigJsonReader configJsonReader(&config, styleManager);
    ConfigOptionReader configOptionReader(&config);

    std::ifstream ifs(expandUserPath("~/.config/jfind/config.json"));
    if (ifs.is_open()) {
        if (!configJsonReader.read(ifs)) {
            fprintf(stderr, "Error in config.json on line %d: %s\n",
                configJsonReader.getErrorLine(),
                configJsonReader.getError().c_str()
            );
            return false;
        }
    }

    if (!configOptionReader.read(argc, argv)) {
        return false;
    }

    createStyles(styleManager);

    if (!config.historyFile.empty()) {
        historyManager = new HistoryManager(config.historyFile);
        historyManager->setHistoryLimit(config.historyLimit);
        historyManager->readHistory();
    }

    return true;
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
    printf("    --history=FILE                Read and write match history to FILE\n");
    printf("    --history-limit=INT           Number of items to store in the history file\n");
    printf("    --prompt=PROMPT               Set the query prompt to PROMPT\n");
    printf("    --query=QUERY                 Set the starting query to QUERY\n");
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
    printf("Input should be piped into JFind for it to be useful. You can try the following example:\n");
    printf("    seq 100 | %s\n", name);
}

int main(int argc, const char **argv) {
    StyleManager *styleManager = userInterface.getStyleManager();

    if (!readConfig(styleManager, argc, argv)) {
        return 1;
    }

    if (isatty(STDIN_FILENO) || config.showHelp) {
        displayHelp(argv[0]);
        return 0;
    }

    ItemCache itemCache;
    itemCache.setSorter(&sorter);
    userInterface.setItemCache(itemCache);
    userInterface.setConfig(&config);

    int fd = open("/dev/tty", O_RDONLY);
    setInputFileNo(fd);
    userInterface.getInputReader()->setFileDescriptor(fd);

    // enable unicode
    setlocale(LC_ALL, "en_US.UTF-8");

    registerResizeCallback([] (int w, int h) {
        userInterface.onResize(w, h);
    });

    styleManager->setOutputFile(stderr);
    userInterface.setOutputFile(stderr);
    setOutputFile(stderr);
    initTerm();
    enableMouse();
    setCursor(true);

    Utf8LineEditor *editor = userInterface.getEditor();
    editor->input(config.query);

    userInterface.drawPrompt();
    userInterface.drawQuery();

    ItemReader itemReader;
    itemReader.setFile(stdin);
    itemReader.setReadHints(config.showHints);

    ThreadCoordinator coordinator;
    coordinator.setItemSorter(&sorter);
    coordinator.setItemReader(itemReader);
    coordinator.setUserInterface(&userInterface);
    coordinator.setHistoryManager(historyManager);
    coordinator.setConfig(&config);
    coordinator.start();

    restoreTerm();

    printResult(userInterface.getSelected());

    return 0;
}

