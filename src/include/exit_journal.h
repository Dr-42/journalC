#ifndef __EXIT_JOURNAL_H__
#define __EXIT_JOURNAL_H__

#include <ncurses.h>

typedef enum {
    INVALID_LOGIN,
    BUFFER_OVERFLOW,
    FILE_ERROR,
    CREATE_USER,
    OPEN_FILE,
} Error;

void exit_journal(Error error, char *message);

#endif // __EXIT_JOURNAL_H__
