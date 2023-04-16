#ifndef __LOGIN_H__
#define __LOGIN_H__
#include <stdbool.h>
#include <ncurses.h>

bool login();
bool new_user(WINDOW *login_win);

#endif // __LOGIN_H__
