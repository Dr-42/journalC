#include "exit_journal.h"
#include <stdlib.h>

void exit_journal(Error error, char *message){
    //Show error in a popup
    endwin();
    WINDOW *error_win = newwin(10, 40, (LINES - 10) / 2, (COLS - 40) / 2);
    refresh();
    box(error_win, 0, 0);
    mvwprintw(error_win, 2, 2, "EXITING JOURNAL");
    switch (error) {
        case INVALID_LOGIN:
            mvwprintw(error_win, 4, 2, "Invalid login");
            break;
        case BUFFER_OVERFLOW:
            mvwprintw(error_win, 4, 2, "Buffer overflow");
            break;
        case FILE_ERROR:
            mvwprintw(error_win, 4, 2, "File error");
            break;
        case CREATE_USER:
            mvwprintw(error_win, 4, 2, "Exit from new user menu");
            break;
        case OPEN_FILE:
            mvwprintw(error_win, 4, 2, "Error opening file");
            break;
        case PASSWORD_MISMATCH:
            mvwprintw(error_win, 4, 2, "Passwords do not match");
            break;
        case EMPTY_FIELD:
            mvwprintw(error_win, 4, 2, "Empty field");
            break;
    }
    mvwprintw(error_win, 6, 2, "%s", message);
    mvwprintw(error_win, 8, 2, "Press any key to exit");
    wrefresh(error_win);
    getch();
    exit(1);
}
