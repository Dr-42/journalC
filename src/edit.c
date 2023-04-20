#include "edit.h"
#include "exit_journal.h"
#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_ENTRY_SIZE 1024*1024

void edit(const char *user){
    // Create window for edit screen
    const int starty = 0;
    const int startx = 1;
    WINDOW *edit_win = newwin(LINES, COLS - 2, starty, startx);
    refresh();

    bool running = true;
    char entry[MAX_ENTRY_SIZE] = {0};
    unsigned int cursor = 0;

    while (running) {

        // Print edit screen
        box(edit_win, 0, 0);

        // Print Software Title
        const char *title = "journalC";
        int title_x = (COLS - strlen(title)) / 2;
        mvwprintw(edit_win, 0, title_x, "%s", title);

        // Main screen is the text input area
        // Pressing F1 will open the help menu
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
        char date_time[50] = {0};
        sprintf(date_time, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        mvwprintw(edit_win, LINES - 1, COLS - 20, "%s", date_time);
        wrefresh(edit_win);

        // Get user input
        int ch = getch();

        bool show_help = false;
        switch (ch) {
            case KEY_F(1):
                // Open help menu
                show_help = !show_help;
                help(show_help);
                break;
            case KEY_F(2):
                // Save entry
                {
                    char filename[50] = {0};
                    sprintf(filename, "%s_%s.ent", user, date_time);
                    FILE *fp = fopen(filename, "w");
                    if (fp == NULL) {
                        exit_journal(FILE_ERROR, "Could not open an entry file");
                    }
                    fprintf(fp, "%s", entry);
                    fclose(fp);
                    for (int i = 0; i < MAX_ENTRY_SIZE; i++){
                        entry[i] = '\0';
                    }
                    cursor = 0;
                }
                break;
            case KEY_F(3):
                // Exit program
                running = false;
                break;
            case KEY_F(4):
                // Clear text input area
                {
                    for (int i = 0; i < MAX_ENTRY_SIZE; i++){
                        entry[i] = '\0';
                    }
                    cursor = 0;
                }
                break;
            case KEY_F(5):
                // Open previous entries
                break;

            // Arrow key controls
            case KEY_UP:
                // Move cursor up
                {
                    bool is_first_line = true;
                    int previous_line_end = 0;
                    for (int i = cursor; i >= 0; i--) {
                        if (entry[i] == '\n') {
                            is_first_line = false;
                            previous_line_end = i - 1;
                            break;
                        }
                    }
                    if (!is_first_line) {
                        int previous_line_length = 0;
                        for (int i = previous_line_end; i >= 0; i--) {
                            if (entry[i] == '\n') {
                                break;
                            } else {
                                previous_line_length++;
                            }
                        }
                        cursor -= previous_line_length + 1;
                        //Check if cursor reached the previous line, else place cursor at end
                        if (cursor > previous_line_end) {
                            cursor = previous_line_end + 1;
                        }
                    }
                }
                break;
            case KEY_DOWN:
                // Move cursor down
                {
                    bool is_last_line = true;
                    int current_line_end = 0;
                    for (int i = cursor; i < strlen(entry); i++){
                        if (entry[i] == '\n'){
                            is_last_line = false;
                            current_line_end = i - 1;
                            break;
                        }
                    }
                    if (!is_last_line){
                        int current_line_length = 0;
                        int next_line_end = 0;
                        for (int i = current_line_end; i >= 0; i--){
                            current_line_length++;
                            if (entry[i] == '\n'){
                                break;
                            }
                        }
                        int numnewline = 0;
                        for (int i = cursor; i <= strlen(entry); i++){
                            if (entry[i] == '\n' || entry[i] == '\0'){
                                numnewline++;
                                if (numnewline == 2){
                                    next_line_end = i;
                                    break;
                                }
                            }
                        }
                        cursor += current_line_length;
                        if (cursor > next_line_end){
                            cursor = next_line_end;
                        }
                    }
                }
                break;
            case KEY_LEFT:
                // Move cursor left
                if (cursor > 0) {
                    cursor--;
                }
                break;
            case KEY_RIGHT:
                // Move cursor right
                if (cursor < strlen(entry)) {
                    cursor++;
                }
                break;

            //Basic Keyboard controls
            case KEY_BACKSPACE:
                //Delete the last character in the current frame
                if (cursor > 0) {
                    for (int i = cursor; i < strlen(entry); i++) {
                        entry[i - 1] = entry[i];
                    }
                    entry[strlen(entry) - 1] = '\0';
                    cursor--;
                }
                break;
            case KEY_ENTER:
                //Type in the curent frame
                if (strlen(entry) < MAX_ENTRY_SIZE) {
                    entry[strlen(entry)] = '\n';
                    cursor++;
                }
                break;
            default:
                //Type in the curent frame at the cursor
                if (strlen(entry) < MAX_ENTRY_SIZE) {
                    for (int i = strlen(entry); i > cursor; i--) {
                        entry[i] = entry[i - 1];
                    }
                    entry[cursor] = ch;
                    cursor++;
                }
                break;
        }
        //clear the screen
        werase(edit_win);
        print_entry(edit_win, entry, cursor);
        wrefresh(edit_win);
    }
}

// Print entry to the screen
void print_entry(WINDOW *edit_win, char *entry, unsigned int cursor){
    int x = 1;
    int y = 1;
    int i = 0;
    while (entry[i] != '\0') {
        if (entry[i] == '\n') {
            y++;
            x = 1;
        } else {
            mvwprintw(edit_win, y, x, "%c", entry[i]);
            x++;
        }
        i++;
    }
    x = 1;
    y = 1;
    // Print cursor
    for (int i = 0; i < cursor; i++) {
        if (entry[i] == '\n') {
            y++;
            x = 1;
        } else {
            x++;
        }
    }
    wattron(edit_win, A_STANDOUT);
    if (entry[cursor] == '\n') {
        mvwprintw(edit_win, y, x, "%c", ' ');
    } else if (entry[cursor] == '\0') {
        mvwprintw(edit_win, y, x, "%c", ' ');
    } else {
        mvwprintw(edit_win, y, x, "%c", entry[cursor]);
    }
    wattroff(edit_win, A_STANDOUT);
}

//Display help window when show_help is true
void help(bool show_help){
    if (show_help) {
        bool running = true;
        while(running){
            // Create window for help menu
            const int starty = 2;
            const int startx = (COLS - 40) / 2;
            WINDOW *help_menu = newwin(9, 40, starty, startx);
            box(help_menu, 0, 0);

            // Print help menu title
            const char *title = "Help";
            int title_x = (40 - strlen(title)) / 2;
            mvwprintw(help_menu, 0, title_x, "%s", title);

            // Print help information for function keys
            mvwprintw(help_menu, 2, 2, "F1: Open Help Menu");
            mvwprintw(help_menu, 3, 2, "F2: Save Entry");
            mvwprintw(help_menu, 4, 2, "F3: Exit Program");
            mvwprintw(help_menu, 5, 2, "F4: Clear Text Input Area");
            mvwprintw(help_menu, 6, 2, "F5: Open Previous Entries");

            // Refresh and show help menu
            wrefresh(help_menu);
            if (getch() == KEY_F(1)) {
                delwin(help_menu);
                running = false;
            }
        }
    }
}
