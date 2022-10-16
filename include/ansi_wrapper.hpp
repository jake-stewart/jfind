#ifndef ANSI_WRAPPER_H
#define ANSI_WRAPPER_H

#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>

static termios initial;
static void (*resize_callback)(int, int) = nullptr;

void useStderr(bool value);

void register_resize_callback(void (*func)(int, int));

void move(int x, int y);
void move_home();
void move_up();

void clear_term();
void clear_til_eol();
void clear_til_eof();

void set_alternate_buffer(bool value);
void set_cursor(bool value);
void set_ctrl_c(bool value);

void init_term(void);
void init_term_simple(void);
void restore_term(void);

#endif
