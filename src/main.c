#include <ncurses.h>
#include <string.h>

void init_curses() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
}

bool login(){
    // Create window for login screen
    int height = 10;
    int width = 40;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    WINDOW *login_win = newwin(height, width, starty, startx);
    refresh();

    // Print login screen
    box(login_win, 0, 0);
    mvwprintw(login_win, 2, 2, "Welcome to My Login System");

    // Input fields
    char username[31] = {0}; // Increased size to accommodate null terminator
    char password[31] = {0}; // Increased size to accommodate null terminator
    int current_field = 0; // 0 for username field, 1 for password field

    // Loop for user input
    int ch;
    bool running = true;
    while (running) {
        // Print username field
        mvwprintw(login_win, 4, 2, "Username: ");
        if (current_field == 0) {
            wattron(login_win, A_STANDOUT); // Highlight current field
            mvwprintw(login_win, 4, 12, "          ");
            mvwprintw(login_win, 4, 12, username);
            wattroff(login_win, A_STANDOUT);
        } else {
            mvwprintw(login_win, 4, 12, username);
        }

        // Print password field
        mvwprintw(login_win, 6, 2, "Password: ");
        if (current_field == 1) {
            wattron(login_win, A_STANDOUT); // Highlight current field
            mvwprintw(login_win, 6, 12, "          ");
            // Print '*' characters for password
            for (int i = 0; i < strlen(password); i++) {
                mvwprintw(login_win, 6, 12 + i, "*");
            }
            wattroff(login_win, A_STANDOUT);
        } else {
            // Print empty field for password
            mvwprintw(login_win, 6, 12, "**********");
        }

        // Print login button
        mvwprintw(login_win, 8, 2, "[Login]");

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
                if (current_field == 0 && strlen(username) < 20) {
                    strncat(username, &ch, 1);
                } else if (current_field == 1 && strlen(password) < 20) {
                    strncat(password, &ch, 1);
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

enum {
    INVALID_LOGIN
}Error;

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

int main() {
    init_curses();
    if (login()){
        edit();
    } else {
        exit_journal(INVALID_LOGIN);
    }
    endwin();
    return 0;
}
