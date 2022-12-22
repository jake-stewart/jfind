#include "../include/ansi_wrapper.hpp"

#define ANSI_ESC "\x1b["

bool in_alternate_buffer = false;
bool ignore_sigint = false;
bool mouse_enabled = false;
int ACTIVE_FILENO = STDOUT_FILENO;
termios orig_termios;

void move(unsigned int x, unsigned int y) {
    printf(ANSI_ESC "%u;%uH", y + 1, x + 1);
}

void move_home() {
    printf(ANSI_ESC "H");
}

void move_up() {
    printf(ANSI_ESC "A");
}

void move_down() {
    printf(ANSI_ESC "B");
}

void move_right() {
    printf(ANSI_ESC "C");
}

void move_left() {
    printf(ANSI_ESC "D");
}

void move_up(unsigned int amount) {
    printf(ANSI_ESC "%uA", amount);
}

void move_down(unsigned int amount) {
    printf(ANSI_ESC "%uB", amount);
}

void move_right(unsigned int amount) {
    printf(ANSI_ESC "%uC", amount);
}

void move_left(unsigned int amount) {
    printf(ANSI_ESC "%uD", amount);
}

void move_up_or_scroll() {
    printf("\x1bM");
}

void move_down_or_scroll() {
    printf("\n");
}

void enable_mouse() {
    if (!mouse_enabled) {
        mouse_enabled = true;
        printf(ANSI_ESC "?1002h" ANSI_ESC "?1015h" ANSI_ESC "?1006h");
    }
}

void disable_mouse() {
    if (mouse_enabled) {
        mouse_enabled = false;
        printf(ANSI_ESC "?1000l");
    }
}

void set_alternate_buffer(bool value) {
    if (in_alternate_buffer == value) {
        return;
    }
    in_alternate_buffer = value;
    printf(ANSI_ESC "?1049%c", value ? 'h' : 'l');
}

void clear_til_eol() {
    printf(ANSI_ESC "K");
}

void clear_til_sof() {
    printf(ANSI_ESC "1J");
}

void clear_til_eof() {
    printf(ANSI_ESC "2J");
}

void clear_term() {
    printf(ANSI_ESC "2J");
}

void set_cursor(bool value) {
    printf(ANSI_ESC "?25%c", value ? 'h' : 'l');
}

void exit_gracefully(int sig) {
    restore_term();
    exit(1);
}

void set_ctrl_c(bool value) {
    ignore_sigint = value;
}

void on_sigint(int sig) {
    if (!ignore_sigint) {
        exit_gracefully(sig);
    }
}

void useStderr(bool value) {
    if (value == (ACTIVE_FILENO == STDERR_FILENO)) {
        return;
    }

    ACTIVE_FILENO = value ? STDERR_FILENO : STDOUT_FILENO;
    FILE *tmp = stdout;
    stdout = stderr;
    stderr = tmp;
}

void on_sig_resize(int sig) {
    winsize ws;
    if (ioctl(ACTIVE_FILENO, TIOCGWINSZ, &ws)) {
        restore_term();
        printf("Failed to query terminal size\n");
        exit(1);
    }
    if (resize_callback != nullptr) {
        resize_callback(ws.ws_col, ws.ws_row);
    }
}

void save_cursor() {
    printf("\x1b" "7");
}

void restore_cursor() {
    printf("\x1b" "8");
}

void refresh() {
    fflush(stdout);
}

void restore_term(void) {
    set_alternate_buffer(true);       // enter alternate buffer
    disable_mouse();
    printf("\x1b[0m");
    clear_term();                     // clean up the buffer
    set_cursor(true);                 // show the cursor
    set_alternate_buffer(false);      // return to the main buffer
    useStderr(false);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);

    signal(SIGWINCH, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGINT, SIG_DFL);
}

// char stdout_buffer[50000];

void init_term(void) {
    // since we're using printf here, which doesn't play nicely
    // with non-canonical mode, we need to turn off buffering.
    // setvbuf(stdout, NULL, _IONBF, 0);
    // setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));

    tcgetattr(STDIN_FILENO, &orig_termios);
    termios raw = orig_termios;

    raw.c_iflag &= ~(ICRNL);         // differentiate newline and linefeed
    raw.c_lflag &= ISIG;             // generate exit signals
    raw.c_lflag &= ~(ECHO | ICANON); // disable echo and cannonical mode

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    signal(SIGWINCH, on_sig_resize);
    signal(SIGTERM, exit_gracefully);
    signal(SIGQUIT, exit_gracefully);
    signal(SIGINT, on_sigint);

    set_alternate_buffer(true);
    clear_term();
    move_home();
    on_sig_resize(0);
}

void register_resize_callback(void (*func)(int, int)) {
    resize_callback = func;
}
