#ifndef __EXIT_JOURNAL_H__
#define __EXIT_JOURNAL_H__

#include <ncurses.h>

typedef enum {
    INVALID_LOGIN
} Error;

void exit_journal(int error);

#endif // __EXIT_JOURNAL_H__
