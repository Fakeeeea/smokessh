//
// Created by aaa on 15/10/2026.
//

#include "terminal.h"

void error(char *msg) {
    printf(msg);
    exit(-1);
}

int get_terminal_size(struct winsize *terminal_size) {
    //if (ioctl(STDOUT_FILENO, TIOCGWINSZ, terminal_size) != 0) {
        //error("Failed fetching terminal size!");
    //}
    if (terminal_size->ws_col >= 256) {
        printf("%d >= 256! A terminal too big leads to performance issues! Cols limited to 256", terminal_size->ws_col);
        terminal_size->ws_col = 256;
    }
    if (terminal_size->ws_row >= 256) {
        printf("%d >= 256! A terminal too big leads to performance issues! Rows limited to 256", terminal_size->ws_row);
        terminal_size->ws_row = 256;
    }
}