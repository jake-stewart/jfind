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

void move(unsigned int x, unsigned int y);
void move_home();
void move_up();

void clear_term();
void clear_til_eol();
void clear_til_sof();
void clear_til_eof();

void move_up();
void move_down();
void move_left();
void move_right();

void move_up(unsigned int amount);
void move_down(unsigned int amount);
void move_left(unsigned int amount);
void move_right(unsigned int amount);

void move_up_or_scroll();
void move_down_or_scroll();

void refresh();

void enable_mouse();
void disable_mouse();

void set_alternate_buffer(bool value);
void set_cursor(bool value);
void set_ctrl_c(bool value);

void init_term(void);
void init_term_simple(void);
void restore_term(void);

#endif
