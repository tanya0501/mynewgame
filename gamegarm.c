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
//ИНИЦИАЛИЗАЦИЯ И РАБОТА С КАТАЛОГОМ ПРЕДМЕТОВ (items.txt)
void set_default_item_catalog(char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    const char *defaults[MAX_ITEMS] = {
        "Пусто",
        "Дерево",
        "Камень",
        "Семена_пшеницы",
        "Железо",
        "Ткань",
        "Еда",
        "Зелье",
        "Инструмент",
        "Самоцветы"
    };

    for (int i = 0; i < MAX_ITEMS; ++i) {
        strncpy(item_names[i], defaults[i], MAX_NAME_LEN - 1);
        item_names[i][MAX_NAME_LEN - 1] = '\0';
    }
}
//Загрузка каталога предметов из файла items.txt. zащита: при ошибке открытия заполняет массив по умолчанию
void load_item_catalog(char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE *file = fopen(ITEMS_FILE, "r");
    if (!file) {
        printf("[Предупреждение] Файл '%s' не найден! Загружен базовый каталог.\n", ITEMS_FILE);
        set_default_item_catalog(item_names);
        return;
    }

    set_default_item_catalog(item_names); // база на случай пустых строк

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        remove_newline(line);
        if (strlen(line) == 0) continue;

        int id;
        char name[MAX_NAME_LEN];
        if (sscanf(line, "%d %31s", &id, name) == 2) {
            if (id >= 0 && id < MAX_ITEMS) {
                strncpy(item_names[id], name, MAX_NAME_LEN - 1);
                item_names[id][MAX_NAME_LEN - 1] = '\0';
            }
        }
    }

    fclose(file);
    printf("[Система] Каталог предметов успешно загружен из '%s'.\n", ITEMS_FILE);
}
