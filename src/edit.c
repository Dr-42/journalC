#include "edit.h"
#include "exit_journal.h"
#include <curses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>

#define AES_IMPLEMENTATION
#include "aes.h"

#define MAX_ENTRY_SIZE 1024
#define MAX_ENTRY_NAME_SIZE 54
#define MAX_NUM_ENTRIES 1000

// Define a struct to pass data to the thread function
typedef struct {
    char *entry;
    unsigned char *encrypted;
    uint8_t *pass;
    int start;
    int end;
} ThreadData;


//Private functions
void display_entry(char* filename, const char *user, const char *password, WINDOW *display_win, char *entry, unsigned int cursor);
void *encrypt_thread(void *arg);
void encrypt_multithreaded(char *entry, unsigned char *encrypted, uint8_t *pass, int num_threads);
void display_previous_entries(const char *user, const char *password, WINDOW *edit_win, char *entry, unsigned int cursor);
void save_entry(const char *user, const char *password, char *entry, unsigned int* cursor, char *date_time);
void print_entry(WINDOW* edit_win, char* entry, unsigned int cursor);
void help(bool show_help);

void edit(const char *user, const char *password){
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
        sprintf(date_time, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
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
                if (strlen(entry) > 0) {
                    save_entry(user, password, entry, &cursor, date_time);
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
                //Entries are saved in the format <user>_<date-time>.ent
                display_previous_entries(user, password, edit_win, entry, cursor);
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

    const uint32_t X_OFFSET = 3;
    const uint32_t Y_OFFSET = 2;
    int x = X_OFFSET;
    int y = Y_OFFSET;
    int i = 0;
    while (entry[i] != '\0') {
        if (entry[i] == '\n') {
            y++;
            x = X_OFFSET;
        } else {
            mvwprintw(edit_win, y, x, "%c", entry[i]);
            x++;
        }
        i++;
    }
    x = X_OFFSET;
    y = Y_OFFSET;
    // Print cursor
    for (int i = 0; i < cursor; i++) {
        if (entry[i] == '\n') {
            y++;
            x = X_OFFSET;
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
            WINDOW *help_menu = newwin(20, 40, starty, startx);
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
            mvwprintw(help_menu, 7, 2, "--------------------------------");
            mvwprintw(help_menu, 8, 2, "Inside Text Input Area:");
            mvwprintw(help_menu, 9, 2, "Arrow Keys: Move Cursor");
            mvwprintw(help_menu, 10, 2, "Backspace: Delete Character");
            mvwprintw(help_menu, 11, 2, "Enter: New Line");
            mvwprintw(help_menu, 12, 2, "Any Other Key: Type Character");
            mvwprintw(help_menu, 13, 2, "--------------------------------");
            mvwprintw(help_menu, 14, 2, "Inside Previous Entries:");
            mvwprintw(help_menu, 15, 2, "Up/Down Arrow Keys: Move Cursor");
            mvwprintw(help_menu, 16, 2, "Right Arrow: Open Entry");
            mvwprintw(help_menu, 17, 2, "Left Arrow: Close Entry");
            mvwprintw(help_menu, 18, 2, "Left Arrow: Close Entries List");

            // Refresh and show help menu
            wrefresh(help_menu);
            if (getch() == KEY_F(1)) {
                delwin(help_menu);
                running = false;
            }
        }
    }
}

void display_previous_entries(const char *user, const char *password, WINDOW *edit_win, char *entry, unsigned int cursor){
    DIR *d;
    struct dirent *dir;
    if (!(d = opendir("entries"))){
        exit_journal(FILE_ERROR, "Could not open directory");
    }
    if (!(dir = readdir(d))){
        exit_journal(FILE_ERROR, "Could not read directory");
    }

    char entries[MAX_NUM_ENTRIES][MAX_ENTRY_NAME_SIZE] = {0};
    int entry_count = 0;

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") == 0) continue;
        if (dir->d_type != DT_DIR) {

            char* total_path = malloc(strlen("entries/") + strlen(dir->d_name) + 1);
            strcpy(total_path, "entries/");
            strcat(total_path, dir->d_name);
            
            //Check if file is an entry
            if (strstr(total_path, ".ent") != NULL){
                //Check if file is for the current user
                if (strstr(total_path, user) != NULL){
                    //Add entry to list
                    strcpy(entries[entry_count], total_path);
                    entry_count++;
                }
            }
            free(total_path);
        }
    }

    closedir(d);

    bool show_selection_popup = true;
    int selected_entry = 0;
    int selected_entry_offset = 0;

    while(show_selection_popup){
        //Create a panel on the left side usiing half vertical space
        WINDOW *popup_win = newwin(LINES - 2, 3 * COLS / 4, 1, 0);
        box(popup_win, 0, 0);
        wrefresh(popup_win);

        //Print entry file names
        //If there are more than LINES - 4 entries, only print the entries that are in the current view
        int num_entries_to_print = LINES - 4;
        for(int i = selected_entry_offset; i < num_entries_to_print; i++){
            #include <stdio.h>

            char username[32];
            char date[11];
            char time[9];
            //entries/admin_2023-04-21 18:44:50.ent
            sscanf(entries[i], "entries/%[^_]_%[^ ] %[^.].ent", username, date, time);

            if (i == selected_entry){
                wattron(popup_win, A_REVERSE);
                mvwprintw(popup_win, i - selected_entry_offset + 1, 1, "Username: %s Date: %s Time: %s", username, date, time);
                wattroff(popup_win, A_REVERSE);
            } else {
                mvwprintw(popup_win, i - selected_entry_offset + 1, 1, "%03d> %s at %s", i+1, date, time);
            }

            //Clear username and date and time
            memset(username, 0, sizeof(username));
            memset(date, 0, sizeof(date));
            memset(time, 0, sizeof(time));

            if (i == entry_count - 1){
                break;
            }
        }

        wrefresh(popup_win);

        //Get user input
        //Arrow keys move the cursor up and down
        //Right arrow opens the selected entry
        //Left arrow exits the popup
        //F5 exits the popup
        int ch = getch();

        switch (ch) {
            case KEY_UP:
                if (selected_entry > 0){
                    selected_entry--;
                    if (selected_entry < selected_entry_offset){
                        selected_entry_offset--;
                    }
                }
                break;
            case KEY_DOWN:
                //The 5 may be related to num entries to print
                if (selected_entry + selected_entry_offset < entry_count - 1){
                    selected_entry++;
                    if (selected_entry > selected_entry_offset + num_entries_to_print - 1){
                        selected_entry_offset++;
                    }
                }
                break;
            case KEY_LEFT:
                show_selection_popup = false;
                break;
            case KEY_RIGHT:
                {
                    bool show_entry_popup = true;
                    while(show_entry_popup){
                        WINDOW *popup_win = newwin(LINES - 2, 3 * COLS / 4, 1, 0);

                        //Print entry
                        display_entry(entries[selected_entry], user, password, popup_win, entry, cursor);

                        wrefresh(popup_win);
                        box(popup_win, 0, 0);
                        wrefresh(popup_win);

                        //Print entry file name on top of popup
                        char username[32];
                        char date[11];
                        char time[9];
                        //entries/admin_2023-04-21 18:44:50.ent
                        sscanf(entries[selected_entry], "entries/%[^_]_%[^ ] %[^.].ent", username, date, time);

                        wattron(popup_win, A_BOLD);
                        mvwprintw(popup_win, 0, 1, "Date: %s Time: %s User: %s", date, time, username);
                        wattroff(popup_win, A_BOLD);
                        wrefresh(popup_win);

                        int ch = getch();
                        switch (ch) {
                            case KEY_LEFT:
                                show_entry_popup = false;
                                break;
                        }
                    }
                }
                break;                        
        }
    }

    entry_count = 0;
    for(int i = 0; i < MAX_NUM_ENTRIES; i++){
        for(int j = 0; j < MAX_ENTRY_NAME_SIZE; j++){
            entries[i][j] = '\0';
        }
    }
    // Clear output area
    int num_lines = LINES - 2;
    for (int i = 0; i < num_lines; i++){
        mvwprintw(edit_win, i + 1, 1, "                                                                                                    ");
    }
    //Clear entry from screen
    for (int i = 0; i < MAX_ENTRY_SIZE; i++){
        entry[i] = '\0';
    }
    // Print entry
    print_entry(edit_win, entry, cursor);
    wrefresh(edit_win);
}


void *encrypt_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int i = data->start; i < data->end; i++) {
        uint8_t *w = aes_init(sizeof(data->pass));

        unsigned char *in = (unsigned char *)data->entry + (i * 16);
        aes_key_expansion(data->pass, w);
        unsigned char *out = data->encrypted + (i * 16);
        aes_cipher(in, out, w);
        aes_free(w);
    }

    return NULL;
}

void encrypt_multithreaded(char *entry, unsigned char *encrypted, uint8_t *pass, int num_threads) {
    int length = (strlen(entry) / 16) + 1;
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].entry = entry;
        thread_data[i].encrypted = encrypted;
        thread_data[i].pass = pass;
        thread_data[i].start = i * (length / num_threads);
        thread_data[i].end = (i == num_threads - 1) ? length : (i + 1) * (length / num_threads);

        pthread_create(&threads[i], NULL, encrypt_thread, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}


void save_entry(const char *user, const char *password, char *entry, unsigned int* cursor, char *date_time){
    //Create entries folder if it doesn't exist
    struct stat st = {0};
    if (stat("entries", &st) == -1) {
        mkdir("entries", 0700);
    }

    //Create entry file
    char filename[60] = {0};
    sprintf(filename, "entries/%s_%s.ent", user, date_time);
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        exit_journal(FILE_ERROR, "Could not open an entry file");
    }

    uint8_t pass[16] = {0};
    for (int i = 0; i < strlen(password); i++){
        pass[i] = password[i];
    }

    unsigned char encrypted[MAX_ENTRY_SIZE] = {0};
    encrypt_multithreaded(entry, encrypted, pass, 4);
    fwrite(encrypted, sizeof(encrypted), 1, fp);
    fclose(fp);
    for (int i = 0; i < MAX_ENTRY_SIZE; i++){
        entry[i] = '\0';
    }
    *cursor = 0;
}

void display_entry(char* filename, const char *user, const char *password, WINDOW *display_win, char *entry, unsigned int cursor){
    //Open file
    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        exit_journal(FILE_ERROR, "Could not open entry file");
    }
    //Read file
    unsigned char file_entry[MAX_ENTRY_SIZE] = {0};
    fread(file_entry, sizeof(char), MAX_ENTRY_SIZE, fp);

    uint8_t pass[16] = {0};
    for (int i = 0; i < strlen(password); i++){
        pass[i] = password[i];
    }
    for (int i = 0; i < ((sizeof(file_entry) / 16) + 1); i++){
        uint8_t *w; // AES expanded key
        w = aes_init(sizeof(pass));

        unsigned char *in = (unsigned char *)file_entry + (i * 16);
        aes_key_expansion(pass, w);
        unsigned char *out = file_entry + (i * 16);
        aes_inv_cipher(in, out, w);
        aes_free(w);
    }
    char* decrypted = (char*)file_entry;
    fclose(fp);
    //Print file
    //Clear entry from screen
    for (int i = 0; i < MAX_ENTRY_SIZE; i++){
        entry[i] = '\0';
    }

    // Copy file_entry to entry
    for (int i = 0; i < strlen(decrypted); i++){
        entry[i] = file_entry[i];
    }
    // Clear output area
    wclear(display_win);

    // Print entry
    mvwprintw(display_win, 0, COLS + 2 - strlen(filename), "%s", filename);
    print_entry(display_win, entry, strlen(entry));
    wrefresh(display_win);
}
