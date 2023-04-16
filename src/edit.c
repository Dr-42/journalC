#include "edit.h"
#include <curses.h>
#include <string.h>
#include <time.h>

void edit(const char *user){
    // Create window for edit screen
    const int starty = 0;
    const int startx = 1;
    WINDOW *edit_win = newwin(LINES, COLS - 2, starty, startx);
    refresh();

    bool running = true;

    while (running) {

        // Print edit screen
        box(edit_win, 0, 0);

        // Print Software Title
        const char *title = "journalC";
        int title_x = (COLS - strlen(title)) / 2;
        mvwprintw(edit_win, 0, title_x, "%s", title);

        // Main screen is the text input area
        // Pressing F2 will save the text to a file
        // Pressing F3 will exit the program
        // Pressing F4 will clear the text input area without saving
        // Pressing F5 will open the previous entries of the current user
        // The entries will open in a pop up and previous entries are not editable
        // There is a bottom status line that will display the current user, time of entry, and date of entry
        // If making a new entry, the time and date will be the current time and date
        // Saving an entry will will lock the entry and the time and date will be the time and date of the entry

        // Print user
        mvwprintw(edit_win, LINES - 1 , 1, "User: %s", user);

        // Print Time and Date
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        mvwprintw(edit_win, LINES - 1, COLS - 20, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);


        wrefresh(edit_win);

        // Get user input
        int ch = getch();

        switch (ch) {
            case KEY_F(2):
                // Save entry
                break;
            case KEY_F(3):
                // Exit program
                running = false;
                break;
            case KEY_F(4):
                // Clear text input area
                break;
            case KEY_F(5):
                // Open previous entries
                break;
            default:
                break;
        }
    }
}
