#include "login.h"
#include "edit.h"
#include "exit_journal.h"
#include "utils.h"
#include <string.h>

int main() {
    init_curses();
    char user[16] = {0};
    char pass[16] = {0};
    if (login(user, pass)){
        edit(user, pass);
    } else {
        exit_journal(INVALID_LOGIN, "Invalid username or password");
    }
    // Wipe the password from memory
    memset(pass, 0, sizeof(pass));
    end_curses();
    return 0;
}
