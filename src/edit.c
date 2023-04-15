#include "edit.h"

void edit(){
    // Create window for edit screen
    int height = 10;
    int width = 40;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    WINDOW *edit_win = newwin(height, width, starty, startx);
    refresh();

    // Print edit screen
    box(edit_win, 0, 0);
    mvwprintw(edit_win, 2, 2, "Welcome to My Edit System");
    mvwprintw(edit_win, 4, 2, "This is the edit screen");
    mvwprintw(edit_win, 6, 2, "Press any key to exit");
    wrefresh(edit_win);

    // Wait for user input
    getch();
}
