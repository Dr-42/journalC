#include "utils.h"
#include <ncurses.h>

void init_curses() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
}

void end_curses() {
    endwin();
}
