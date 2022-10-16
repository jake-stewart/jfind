#include "../include/ansi_wrapper.hpp"

#define ANSI_ESC "\x1b["

void move(int x, int y) {
    printf(ANSI_ESC "%u;%uH", y + 1, x + 1);
}

void move_home() {
    printf(ANSI_ESC "H");
}

void set_alternate_buffer(bool value) {
    printf(ANSI_ESC "?1049%c", value ? 'h' : 'l');
}

void clear_term() {
    printf(ANSI_ESC "2J");
}

void set_cursor(bool value) {
    printf(ANSI_ESC "?25%c", value ? 'h' : 'l');
}

int start_x, start_y;

void exit_gracefully(int sig) {
    restore_term();
    exit(1);
}

bool ignore_sigint = false;
void set_ctrl_c(bool value) {
    ignore_sigint = value;
}

void on_sigint(int sig) {
    if (!ignore_sigint) {
        exit_gracefully(sig);
    }
}

int ACTIVE_FILENO = STDOUT_FILENO;

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

termios orig_termios;

void restore_term(void) {
    set_alternate_buffer(true);       // enter alternate buffer
    clear_term();                     // clean up the buffer
    set_cursor(true);                 // show the cursor
    set_alternate_buffer(false);      // return to the main buffer
    useStderr(false);

    // restore original termios
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void init_term(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    termios raw = orig_termios;

    /* raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); */
    /* raw.c_oflag &= ~(OPOST); */
    /* raw.c_cflag |= (CS8); */
    /* raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); */

    raw.c_iflag &= ~(ICRNL);  // differentiate newline and linefeed
    raw.c_lflag &= ISIG;  // generate exit signals
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    signal(SIGWINCH, on_sig_resize);
    signal(SIGTERM, exit_gracefully);
    signal(SIGQUIT, exit_gracefully);
    signal(SIGKILL, exit_gracefully);
    signal(SIGINT, on_sigint);

    set_alternate_buffer(true);
    clear_term();
    move_home();
    on_sig_resize(0);
}

void register_resize_callback(void (*func)(int, int)) {
    resize_callback = func;
}
