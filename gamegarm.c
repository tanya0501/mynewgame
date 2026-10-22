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
//Запрос имени персонажа у пользователя при старте.
void init_farmer_name(char *farmer_name) {
    printf("==================================================\n");
    printf("        ДОБРО ПОЖАЛОВАТЬ В СИМУЛЯТОР ФЕРМЫ!       \n");
    printf("==================================================\n");
    printf("Введите имя вашего фермера: ");

    if (fgets(farmer_name, MAX_FARMER_NAME_LEN, stdin)) {
        remove_newline(farmer_name);
        if (strlen(farmer_name) == 0) {
            strncpy(farmer_name, "Безымянный фермер", MAX_FARMER_NAME_LEN - 1);
        }
    } else {
        strncpy(farmer_name, "Безымянный фермер", MAX_FARMER_NAME_LEN - 1);
    }
    farmer_name[MAX_FARMER_NAME_LEN - 1] = '\0';
}

//функции пунктов меню
void print_menu(const char *farmer_name) {
    printf("\n==================================================\n");
    printf(" Ферма игрока: %s\n", farmer_name);
    printf("==================================================\n");
    printf(" [0] Выход\n");
    printf(" [1] Посмотреть на часы\n");
    printf(" [2] Промотать время (Поработать)\n");
    printf(" [3] Посмотреть инвентарь\n");
    printf(" [4] Положить предмет в слот\n");
    printf(" [5] Выбросить предмет\n");
    printf(" [6] Ревизия ресурсов (Вариант 1)\n");
    printf(" [7] Поиск предмета в рюкзаке по названию\n");
    printf(" [8] Записать состояние в дневник фермера (diary.txt)\n");
    printf(" [9] Расшифровать старые записи (Вариант: Цезарь / Инверсия)\n");
    printf("--------------------------------------------------\n");
}
void show_time(int day, int hour) {
    printf("\nТекущее время: День %d, %02d:00\n", day, hour);
}

void pass_time(int *day, int *hour) {
    int work_hours = get_valid_int(0, 1000, "Сколько часов вы хотите потратить на работу? ");
    *hour += work_hours;

    while (*hour >= HOURS_IN_DAY) {
        *hour -= HOURS_IN_DAY;
        (*day)++;
    }

    printf("Время промотано. Теперь: День %d, %02d:00\n", *day, *hour);
}
void show_inventory(const int *inventory, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    printf("\n--- СОДЕРЖИМОЕ ИНВЕНТАРЯ ---\n");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = inventory[i];
        const char *name = (id >= 0 && id < MAX_ITEMS) ? item_names[id] : "Неизвестно";
        printf("Слот %d: [%d] - %s\n", i, id, name);
    }
}
void put_item(int *inventory, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int slot = get_valid_int(0, INVENTORY_SIZE - 1, "Введите индекс слота (0-9): ");
    int item_id = get_valid_int(0, MAX_ITEMS - 1, "Введите ID предмета (0-9): ");

    inventory[slot] = item_id;
    printf("Предмет \"%s\" (ID %d) успешно помещён в слот %d.\n", item_names[item_id], item_id, slot);
}
void drop_item(int *inventory, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int slot = get_valid_int(0, INVENTORY_SIZE - 1, "Введите индекс слота для удаления (0-9): ");
    int old_id = inventory[slot];

    inventory[slot] = EMPTY_SLOT;
    printf("Слот %d очищен. Предмет \"%s\" (ID %d) выброшен.\n", slot, item_names[old_id], old_id);
}
void audit_resources(const int *inventory) {
    int target_id = get_valid_int(0, MAX_ITEMS - 1, "Введите ID предмета для ревизии (0-9): ");
    int count = 0;

    printf("Позиции слотов: ");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (inventory[i] == target_id) {
            printf("%d ", i);
            count++;
        }
    }

    if (count > 0) {
        printf("\nВсего найдено предметов ID %d: %d шт.\n", target_id, count);
    } else {
        printf("нет (таких предметов в инвентаре нет).\n");
    }
}

void search_item_by_name(const int *inventory, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    char search_name[MAX_NAME_LEN];
    printf("Введите название предмета для поиска (например, Дерево): ");

    if (!fgets(search_name, sizeof(search_name), stdin)) return;
    remove_newline(search_name);

    int found_id = -1;
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (strcmp(item_names[i], search_name) == 0) {
            found_id = i;
            break;
        }
    }

    if (found_id == -1) {
printf("[Результат] Предмет с названием \"%s\" не найден в каталоге!\n", search_name);
        return;
    }
    printf("[Каталог] Найден ID: %d (\"%s\")\n", found_id, search_name);
    int count = 0;
    printf("Слоты в инвентаре: ");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (inventory[i] == found_id) {
            printf("%d ", i);
            count++;
        }
    }
    if (count == 0) {
        printf("этого предмета сейчас нет в рюкзаке.\n");
    } else {
        printf("\nВсего в рюкзаке: %d шт.\n", count);
    }
}
void save_diary(const char *farmer_name, int day, int hour, const int *inventory, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE *diary = fopen(DIARY_FILE, "a");
    if (!diary) {
        printf("[Ошибка] Не удалось открыть или создать файл дневника '%s'!\n", DIARY_FILE);
        return;
    }

    //Вывод сводки на экран 
    printf("\n--- СОХРАНЕНИЕ В ДНЕВНИК ФЕРМЕРА ---\n");
    printf("Фермер: %s | День %d | Время %02d:00\n", farmer_name, day, hour);
    printf("Содержимое рюкзака:\n");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = inventory[i];
        printf("  Слот %d: [%d] - %s\n", i, id, item_names[id]);
    }

    //Дублирование записи в файл diary.txt 
    fprintf(diary, "--- ЗАПИСЬ В ДНЕВНИКЕ ---\n");
    fprintf(diary, "Фермер: %s | День %d | Время %02d:00\n", farmer_name, day, hour);
    fprintf(diary, "Содержимое рюкзака:\n");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = inventory[i];
        fprintf(diary, "  Слот %d: [%d] - %s\n", i, id, item_names[id]);
    }
    fprintf(diary, "-------------------------\n\n");

    fclose(diary);
    printf("[Успешно] Состояние зафиксировано на экране и записано в '%s'!\n", DIARY_FILE);
}
//ЗАДАНИЕ ПО ВАРИАНТУ
void check_crafting_recipes(const int *inventory, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE *fin = fopen(INPUT_FILE, "r");
    if (!fin) {
        printf("[Ошибка] Файл с рецептами '%s' не найден! Создайте файл '%s'.\n", INPUT_FILE, INPUT_FILE);
        return;
    }

    FILE *fout = fopen(OUTPUT_FILE, "w");
    if (!fout) {
        printf("[Ошибка] Не удалось создать файл для записи '%s'!\n", OUTPUT_FILE);
        fclose(fin);
        return;
    }

    //Считаем фактическое количество каждого ресурса у игрока 
    int available_resources[MAX_ITEMS] = {0};
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = inventory[i];
        if (id > EMPTY_SLOT && id < MAX_ITEMS) {
            available_resources[id]++;
        }
    }

    printf("\n==================================================\n");
    printf("   ПРОВЕРКА РЕЦЕПТОВ КРАФТА (%s -> %s)\n", INPUT_FILE, OUTPUT_FILE);
    printf("==================================================\n");

    char line[BUFFER_SIZE];
    int recipe_count = 0;

    while (fgets(line, sizeof(line), fin)) {
        remove_newline(line);
        if (strlen(line) == 0) continue;

        //Ищем символ двоеточия ':' разделяющий название предмета и список ID
        char *colon_ptr = strchr(line, ':');
        if (!colon_ptr) {
            continue; //Пропуск некорректных строк
        }

        //Выделяем название крафтового предмета 
        *colon_ptr = '\0';
        char recipe_name[MAX_NAME_LEN];
        strncpy(recipe_name, line, MAX_NAME_LEN - 1);
        recipe_name[MAX_NAME_LEN - 1] = '\0';

       //Убираем возможные пробелы на конце имени 
        int name_len = strlen(recipe_name);
        while (name_len > 0 && recipe_name[name_len - 1] == ' ') {
            recipe_name[name_len - 1] = '\0';
            name_len--;
        }
 //2. Считаем требуемые компоненты для этого конкретного рецепта 
        int required_resources[MAX_ITEMS] = {0};
        char *token_str = colon_ptr + 1;
        char *token = strtok(token_str, " \t");

        while (token != NULL) {
            int item_id = atoi(token);
            if (item_id > EMPTY_SLOT && item_id < MAX_ITEMS) {
                required_resources[item_id]++;
            }
            token = strtok(NULL, " \t");
        }

        //3. Проверяем, хватает ли ресурсов у игрока 
        int can_craft = 1;
        char missing_desc[BUFFER_SIZE] = "";

        for (int id = 1; id < MAX_ITEMS; ++id) {
            if (required_resources[id] > available_resources[id]) {
                can_craft = 0;
                int diff = required_resources[id] - available_resources[id];

                char part[64];
                snprintf(part, sizeof(part), "%d %s, ", diff, item_names[id]);
                strncat(missing_desc, part, sizeof(missing_desc) - strlen(missing_desc) - 1);
            }
        }
