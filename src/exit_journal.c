#include "exit_journal.h"

void exit_journal(int error){
    //Show error in a popup
    endwin();
    WINDOW *error_win = newwin(10, 40, (LINES - 10) / 2, (COLS - 40) / 2);
    refresh();
    box(error_win, 0, 0);
    mvwprintw(error_win, 2, 2, "Error");
    if (error == INVALID_LOGIN){
        mvwprintw(error_win, 4, 2, "Invalid login credentials");
    }
    mvwprintw(error_win, 6, 2, "Press any key to exit");
    wrefresh(error_win);
    getch();
}
