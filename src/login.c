#include "login.h"
#include "exit_journal.h"

#include <curses.h>
#include <string.h>
#include <ncurses.h>

#define LOGIN_SCREEN_WIDTH 60
#define LOGIN_SCREEN_HEIGHT 10

bool login(){
    // Create window for login screen
    int starty = (LINES - LOGIN_SCREEN_HEIGHT) / 2;
    int startx = (COLS - LOGIN_SCREEN_WIDTH) / 2;
    WINDOW *login_win = newwin(LOGIN_SCREEN_HEIGHT, LOGIN_SCREEN_WIDTH, starty, startx);
    refresh();

    // Print login screen
    box(login_win, 0, 0);
    mvwprintw(login_win, 2, 2, "Welcome to My Login System");

    // Input fields for username and password
    // secure zero initialize the buffers
    char username[32] = {0};
    char password[32] = {0};
    int current_field = 0; // 0 for username field, 1 for password field

    // Loop for user input
    int ch;
    bool running = true;
    while (running) {
        // Print username field
        const char *login_button = "[Login]";
        const char *username_field = "                               ";
        const char *password_field = "*******************************";

        //X Center the fields
        int username_field_x = (LOGIN_SCREEN_WIDTH - strlen(username_field)) / 2;
        int password_field_x = (LOGIN_SCREEN_WIDTH - strlen(password_field)) / 2;
        int login_button_x = (LOGIN_SCREEN_WIDTH - strlen(login_button)) / 2;

        if (current_field == 0) {
            wattron(login_win, A_STANDOUT); // Highlight current field
            mvwprintw(login_win, 4, username_field_x, "%s", username_field);
            // Print username at center of field
            mvwprintw(login_win, 4 , username_field_x + (strlen(username_field) - strlen(username)) / 2, "%s", username);
            wattroff(login_win, A_STANDOUT);
        } else {
            wattron(login_win, A_DIM); // Dim other fields
            mvwprintw(login_win, 4 , username_field_x + (strlen(username_field) - strlen(username)) / 2, "%s", username);
            wattroff(login_win, A_DIM);
        }

        // Print password field
        if (current_field == 1) {
            wattron(login_win, A_STANDOUT); // Highlight current field
            mvwprintw(login_win, 6, password_field_x, "%s", password_field);
            // Print '*' characters for password
            for (int i = 0; i < strlen(password); i++) {
                mvwprintw(login_win, 6, password_field_x + i, "*");
            }
            wattroff(login_win, A_STANDOUT);
        } else {
            // Print empty field for password
            mvwprintw(login_win, 6, password_field_x, "%s", password_field);
        }

        // Print login button
        mvwprintw(login_win, 8, login_button_x, "%s", login_button);

        // Refresh window
        wrefresh(login_win);

        // Get user input
        ch = getch();
        switch (ch) {
            case KEY_DOWN: // Move to next field
                current_field = (current_field + 1) % 2;
                break;
            case KEY_UP: // Move to previous field
                current_field = (current_field + 1) % 2;
                break;
            case '\t': // Move to next field on Tab key
                current_field = (current_field + 1) % 2;
                break;
            case KEY_BACKSPACE:
            case KEY_DC:
                if (current_field == 0 && strlen(username) > 0) {
                    username[strlen(username) - 1] = '\0';
                } else if (current_field == 1 && strlen(password) > 0) {
                    password[strlen(password) - 1
                        ] = '\0';
                }
                break;
            case '\n':
            case KEY_ENTER: // Submit login credentials
                running = false;
                break;
            default: // Append character to current field
                if (current_field == 0 && strlen(username) < 31) {
                    strncat(username, (const char*)&ch, 1);
                } else if (current_field == 1 && strlen(password) < 31) {
                    strncat(password, (const char*)&ch, 1);
                } else if (current_field == 0 && strlen(username) > 31) {
                    exit_journal(BUFFER_OVERFLOW);
                } else if (current_field == 1 && strlen(password) > 31) {
                    exit_journal(BUFFER_OVERFLOW);
                }
                break;
        }
    }

    // Check if login credentials are valid
    if (strcmp(username, "admin") == 0 && strcmp(password, "password") == 0) {
        return true;
    } else {
        return false;
    }
}
