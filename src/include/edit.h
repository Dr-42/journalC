#ifndef __EDIT_H__
#define __EDIT_H__
#include <ncurses.h>

void edit(const char* user);
void help(bool show_help);
void print_entry(WINDOW* edit_win, char* entry, unsigned int cursor);

#endif // __EDIT_H__
