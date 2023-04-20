#include "login.h"
#include "exit_journal.h"

#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <openssl/sha.h>

#define LOGIN_SCREEN_WIDTH 60
#define LOGIN_SCREEN_HEIGHT 12

#define SHA256_BLOCK_SIZE 32

void new_user();
void char_buffer_fill(char *buffer, int size, char c);

bool login(char *user, char *pass){
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
    char username[SHA256_BLOCK_SIZE] = {0};
    char password[SHA256_BLOCK_SIZE] = {0};
    int current_field = 0; // 0 for username field, 1 for password field

    // Loop for user input
    int ch;
    bool running = true;
    while (running) {
        // Print username field
        const char *login_button = "[Login]";
        // Username and password fields are 31 characters long
        char username_field[SHA256_BLOCK_SIZE];
        char password_field[SHA256_BLOCK_SIZE];

        char_buffer_fill(username_field, SHA256_BLOCK_SIZE, ' ');
        char_buffer_fill(password_field, SHA256_BLOCK_SIZE, '*');

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

        //Prompt at the bottom of the screen
        mvwprintw(login_win, 11, 2, "Press F3 for new user");

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

            case KEY_F(3): // New user
                running = false;
                new_user();

            default: // Append character to current field
                if (current_field == 0 && strlen(username) < 31) {
                    strncat(username, (const char*)&ch, 1);
                } else if (current_field == 1 && strlen(password) < 31) {
                    strncat(password, (const char*)&ch, 1);
                } else if (current_field == 0 && strlen(username) > 31) {
                    exit_journal(BUFFER_OVERFLOW, "Username too long");
                } else if (current_field == 1 && strlen(password) > 31) {
                    exit_journal(BUFFER_OVERFLOW, "Password too long");
                }
                break;
        }
    }

    // Check if login credentials are valid
    // Hash password
    unsigned char hash[SHA256_BLOCK_SIZE];
    SHA256((const unsigned char*)password, strlen(password), hash);

    // Check users.hash file for matches
    FILE *users_hash_file = fopen("users.hash", "r");
    if (users_hash_file == NULL) {
        exit_journal(OPEN_FILE, "No previous users found. Please create a new user.");
    }

    // Read each line of the file
    char line[32 + SHA256_BLOCK_SIZE * 2 + 1];
    bool found = false;

    while (fgets(line, sizeof(line), users_hash_file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;

        // Split line into username and hash
        char *username_hash = strtok(line, " ");
        char *hash_str = strtok(NULL, " ");

        // Convert hash string to bytes
        unsigned char hash_bytes[SHA256_BLOCK_SIZE];
        for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
            sscanf(hash_str + (i * 2), "%2hhx", &hash_bytes[i]);
        }

        // Compare username and hash
        // If match, set found to true
        if (strcmp(username, username_hash) == 0 && memcmp(hash, hash_bytes, SHA256_BLOCK_SIZE) == 0) {
            strcpy(user, username);
            strcpy(pass, password);
            found = true;
            running = false;
            break;
        }
    }

    // Close file
    // If not found, print error message
    fclose(users_hash_file);
    if (!found) {
        mvwprintw(login_win, 10, 2, "Invalid username or password");
        wrefresh(login_win);
        return false;
    }
    else {
        return true;
    }
}

void new_user() {
    // Create window for login screen
    // Close previous window
    int starty = (LINES - LOGIN_SCREEN_HEIGHT) / 2;
    int startx = (COLS - LOGIN_SCREEN_WIDTH) / 2;
    WINDOW *new_user_win = newwin(LOGIN_SCREEN_HEIGHT, LOGIN_SCREEN_WIDTH, starty, startx);
    refresh();

    // Print login screen
    box(new_user_win, 0, 0);
    mvwprintw(new_user_win, 2, 2, "Create New User                            ");

    // Input fields for username and password
    // secure zero initialize the buffers
    char username[32] = {0};
    char password[32] = {0};
    char password_confirm[32] = {0};
    int current_field = 0; // 0 for username field, 1 for password field

    // Loop for user input
    int ch;
    bool running = true;
    while (running) {
        // Print username field
        const char *new_user_button = "[Create User]";
        char username_field[SHA256_BLOCK_SIZE];
        char password_field[SHA256_BLOCK_SIZE];

        char_buffer_fill(username_field, SHA256_BLOCK_SIZE, ' ');
        char_buffer_fill(password_field, SHA256_BLOCK_SIZE, '*');

        //X Center the fields
        int username_field_x = (LOGIN_SCREEN_WIDTH - strlen(username_field)) / 2;
        int password_field_x = (LOGIN_SCREEN_WIDTH - strlen(password_field)) / 2;
        int password_confirm_field_x = (LOGIN_SCREEN_WIDTH - strlen(password_field)) / 2;
        int login_button_x = (LOGIN_SCREEN_WIDTH - strlen(new_user_button)) / 2;

        if (current_field == 0) {
            wattron(new_user_win, A_STANDOUT); // Highlight current field
            mvwprintw(new_user_win, 4, username_field_x, "%s", username_field);
            // Print username at center of field
            mvwprintw(new_user_win, 4 , username_field_x + (strlen(username_field) - strlen(username)) / 2, "%s", username);
            wattroff(new_user_win, A_STANDOUT);
        } else {
            wattron(new_user_win, A_DIM); // Dim other fields
            mvwprintw(new_user_win, 4 , username_field_x + (strlen(username_field) - strlen(username)) / 2, "%s", username);
            wattroff(new_user_win, A_DIM);
        }

        // Print password field
        if (current_field == 1) {
            wattron(new_user_win, A_STANDOUT); // Highlight current field
            mvwprintw(new_user_win, 6, password_field_x, "%s", password_field);
            // Print '*' characters for password
            for (int i = 0; i < strlen(password); i++) {
                mvwprintw(new_user_win, 6, password_field_x + i, "*");
            }
            wattroff(new_user_win, A_STANDOUT);
        } else {
            // Print empty field for password
            mvwprintw(new_user_win, 6, password_field_x, "%s", password_field);
        }
        // Print confirm password field
        if (current_field == 2) {
            wattron(new_user_win, A_STANDOUT); // Highlight current field
            mvwprintw(new_user_win, 8, password_confirm_field_x, "%s", password_field);
            // Print '*' characters for password
            for (int i = 0; i < strlen(password_confirm); i++) {
                mvwprintw(new_user_win, 8, password_confirm_field_x + i, "*");
            }
            wattroff(new_user_win, A_STANDOUT);
        } else {
            // Print empty field for password
            mvwprintw(new_user_win, 8, password_confirm_field_x, "%s", password_field);
        }

        // Print login button
        mvwprintw(new_user_win, 10, login_button_x, "%s", new_user_button);

        // Refresh window
        wrefresh(new_user_win);

        // Get user input
        ch = getch();
        switch (ch) {
            case KEY_DOWN: // Move to next field
                current_field = (current_field + 1) % 3;
                break;
            case KEY_UP: // Move to previous field
                current_field = (current_field + 1) % 3;
                break;
            case '\t': // Move to next field on Tab key
                current_field = (current_field + 1) % 3;
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
                } else if (current_field == 2 && strlen(password_confirm) < 31) {
                    strncat(password_confirm, (const char*)&ch, 1);
                } else if (current_field == 0 && strlen(username) > 31) {
                    exit_journal(BUFFER_OVERFLOW, "Username too long");
                } else if (current_field == 1 && strlen(password) > 31) {
                    exit_journal(BUFFER_OVERFLOW, "Password too long");
                } else if (current_field == 2 && strlen(password_confirm) > 31) {
                    exit_journal(BUFFER_OVERFLOW, "Password too long");
                }
                break;
        }
    }

    if (strlen(username) == 0) {
        exit_journal(EMPTY_FIELD, "Username cannot be empty");
    }

    if (strlen(password) == 0) {
        exit_journal(EMPTY_FIELD, "Password cannot be empty");
    }

    if (strcmp(password, password_confirm) != 0) {
        exit_journal(PASSWORD_MISMATCH, "The passwords do not match");
        printf("%s, %s", password, password_confirm);
    }

    // Save new user to file with sha256 hash of password
    FILE *fp = fopen("users.hash", "a");
    if (fp == NULL) {
        exit_journal(FILE_ERROR, "Could not open users.hash file");
    }

    // Hash password
    unsigned char hash[SHA256_BLOCK_SIZE];
    SHA256((const unsigned char*)password, strlen(password), hash);

    // Print username and hash to file
    fprintf(fp, "%s ", username);
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        fprintf(fp, "%02x", hash[i]);
    }
    fprintf(fp, "%s", "\n");
    // Close file
    fclose(fp);
    exit_journal(CREATE_USER, "New user created");
}

void char_buffer_fill(char *buffer, int size, char c) {
    for (int i = 0; i < size - 1; i++) {
        buffer[i] = c;
    }
    buffer[size - 1] = '\0';
}

