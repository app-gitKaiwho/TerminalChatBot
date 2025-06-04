#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define INPUT_MAX_LEN 255
#define HISTORY_MAX_LINES 100

void animation(WINDOW *animation_win, int max_x, int max_y) {
    static int x = 0;
    const static char c = 'o';
    const static char s = '-';

    box(animation_win, 0, 0);
    mvwaddch(animation_win, 1, x, s);
    x = (x + 1) % (max_x - 1);
    mvwaddch(animation_win, 1, x, c);
    wrefresh(animation_win);
}

void textbox(WINDOW *dialog_win, char *input, int input_pos) {
    static const char prompt[] = "You : ";

    werase(dialog_win);
    mvwprintw(dialog_win, 1, 1, "%s%s", prompt, input);
    box(dialog_win, 0, 0);
    wrefresh(dialog_win);
}

void update_history(char history[HISTORY_MAX_LINES][INPUT_MAX_LEN + 50], int *history_count, const char *message) {
    if (*history_count < HISTORY_MAX_LINES) {
        strcpy(history[*history_count], message);
        (*history_count)++;
    } else {
        for (int i = 1; i < HISTORY_MAX_LINES; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[HISTORY_MAX_LINES - 1], message);
    }
}

void display_history(WINDOW *history_win, char history[HISTORY_MAX_LINES][INPUT_MAX_LEN + 50], int history_count, int history_offset, int history_height) {
    werase(history_win);
    box(history_win, 0, 0);

    for (int i = 0; i < history_height - 2 && i + history_offset < history_count; i++) {
        mvwprintw(history_win, i + 1, 1, "%s", history[i + history_offset]);
    }

    wrefresh(history_win);
}

int main() {
    int max_y, max_x;
    int input_pos = 0;
    const int delay = 10000;
    const int dialog_height = 5;
    const int history_height = 50;
    const int animation_height = 3;
    char input[INPUT_MAX_LEN] = "";
    char history[HISTORY_MAX_LINES][INPUT_MAX_LEN + 50] = {0};
    int history_count = 0;
    int history_offset = 0;

    initscr();
    timeout(0);
    curs_set(0);
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, max_y, max_x);
    WINDOW *animation_win = newwin(animation_height, max_x, 0, 0);
    WINDOW *history_win = newwin(history_height, max_x, animation_height, 0);
    WINDOW *dialog_win = newwin(dialog_height, max_x, history_height + animation_height, 0);

    while (1) {
        int ch = getch();
        if (ch != ERR) {
            if (ch == '\n') {
                if (strcmp(input, "quit") == 0)
                    break;

                char user_message[INPUT_MAX_LEN + 50];
                snprintf(user_message, sizeof(user_message), "You : %s", input);
                update_history(history, &history_count, user_message);

                char server_message[INPUT_MAX_LEN + 50];
                snprintf(server_message, sizeof(server_message), "Server : Response to '%s'", input);
                update_history(history, &history_count, server_message);

                if (history_count > history_height - 2) {
                    history_offset = history_count - (history_height - 2);
                } else {
                    history_offset = 0;
                }

                memset(input, 0, sizeof(input));
                input_pos = 0;
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                if (input_pos > 0)
                    input[--input_pos] = '\0';
            } else if (ch == KEY_UP) {
                if (history_offset > 0)
                    history_offset--;
            } else if (ch == KEY_DOWN) {
                if (history_offset < history_count - (history_height - 2))
                    history_offset++;
            } else if (input_pos < INPUT_MAX_LEN - 1 && ch >= 32 && ch <= 126) {
                input[input_pos++] = ch;
                input[input_pos] = '\0';
            }
        }
        animation(animation_win, max_x, max_y);
        textbox(dialog_win, input, input_pos);
        display_history(history_win, history, history_count, history_offset, history_height);
        usleep(delay);
    }

    delwin(dialog_win);
    delwin(history_win);
    delwin(animation_win);
    endwin();
    return 0;
}