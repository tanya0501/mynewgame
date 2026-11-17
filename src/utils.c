#include <stdio.h>
#include <string.h>
#include "utils.h"

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void remove_newline(char *str) {
    if (!str) return;
    str[strcspn(str, "\r\n")] = '\0';
}

int get_valid_int(int min_val, int max_val, const char *prompt) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            clear_input_buffer();
            if (value >= min_val && value <= max_val) {
                return value;
            }
            printf("[Ошибка] Число должно быть в диапазоне от %d до %d.\n", min_val, max_val);
        } else {
            clear_input_buffer();
            printf("[Ошибка] Некорректный ввод! Введите целое число.\n");
        }
    }
}
