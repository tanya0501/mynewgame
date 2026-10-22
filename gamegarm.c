#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INVENTORY_SIZE      10
#define MAX_ITEMS           10
#define MAX_NAME_LEN        32
#define MAX_FARMER_NAME_LEN 32
#define HOURS_IN_DAY        24
#define BUFFER_SIZE         256

#define ITEMS_FILE          "items.txt"
#define DIARY_FILE          "diary.txt"
#define INPUT_FILE          "input.txt"
#define OUTPUT_FILE         "output.txt"

/* ID предметов по умолчанию */
#define EMPTY_SLOT 0
#define ITEM_WOOD  1
#define ITEM_STONE 2
#define ITEM_SEEDS 3
#define ITEM_IRON  4
#define ITEM_CLOTH 5
#define ITEM_FOOD  6

//ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ВВОДА/ВЫВОДА

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void remove_newline(char *str) {
    if (!str) return;
    str[strcspn(str, "\r\n")] = '\0';
}

//Безопасное считывание целого числа в заданном диапазоне [min_val, max_val]
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
