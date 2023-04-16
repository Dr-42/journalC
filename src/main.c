#include "login.h"
#include "edit.h"
#include "exit_journal.h"
#include "utils.h"

int main() {
    init_curses();
    if (login()){
        edit();
    } else {
        exit_journal(INVALID_LOGIN, "Invalid username or password");
    }
    end_curses();
    return 0;
}
