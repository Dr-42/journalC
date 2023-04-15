#ifndef __EXIT_JOURNAL_H__
#define __EXIT_JOURNAL_H__

#include <ncurses.h>

typedef enum {
    INVALID_LOGIN,
    BUFFER_OVERFLOW,
} Error;

void exit_journal(Error error);

#endif // __EXIT_JOURNAL_H__
