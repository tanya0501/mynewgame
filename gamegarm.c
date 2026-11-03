#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAP_MIN_SIZE 5
#define MAP_MAX_SIZE 30
#define FOREST_MIN_OBSTACLE_PERCENT 15
#define FOREST_MAX_OBSTACLE_PERCENT 30

#define DUNGEON_MODE 1
#define FOREST_MODE 2

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

typedef struct {
	int rows;
	int cols;
	char *cells;
	int player_row;
	int_player_col;
	int_depth;
	int edge_transitiond;
	int mode;
} GameMap;

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
    printf(" [9] Расшифровать старые записи \n");
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

        //Убираем последнюю запятую и пробел, если есть 
        int missing_len = strlen(missing_desc);
        if (missing_len >= 2 && missing_desc[missing_len - 2] == ',') {
            missing_desc[missing_len - 2] = '\0';
        }

        //4. Формируем строку вердикта 
        char result_line[BUFFER_SIZE];
        if (can_craft) {
            snprintf(result_line, sizeof(result_line), "%s: [ДОСТУПНО]", recipe_name);
        } else {
            snprintf(result_line, sizeof(result_line), "%s: [НЕ ХВАТАЕТ РЕСУРСОВ] (Недостает: %s)", recipe_name, missing_desc);
        }

        //Выводим результат одновременно на экран И в файл output.txt 
        printf("%s\n", result_line);
        fprintf(fout, "%s\n", result_line);

        recipe_count++;
    }

    if (recipe_count == 0) {
        printf("[Предупреждение] В файле '%s' не найдено ни одного рецепта!\n", INPUT_FILE);
        fprintf(fout, "В файле рецептов нет данных.\n");
    }

    printf("==================================================\n");
    fclose(fin);
    fclose(fout);
    printf("[Успешно] Все результаты зафиксированы на экране и в '%s'.\n", OUTPUT_FILE);
}
//карта
void free_map(GameMap *map) {
    if (map != NULL) {
        free(map->cells);
        map->cells = NULL;
    }
}
int map_index(const GameMap *map, int row, int col) {
    return row * map->cols + col;
}
int is_blocked(char cell) {
    return cell == '#' || cell == 'T' || cell == 'R';
}
void print_map(const GameMap *map) {
    printf("\n--- КАРТА ---\n");

    for (int row = 0; row < map->rows; ++row) {
        for (int col = 0; col < map->cols; ++col) {
            if (row == map->player_row && col == map->player_col) {
                putchar('@');
            } else {
                putchar(map->cells[map_index(map, row, col)]);
            }
        }
        putchar('\n');
    }

    if (map->mode == DUNGEON_MODE) {
        printf("Этаж: %d\n", map->depth);
        printf("WASD — движение, Q — домой на лестнице U\n");
    } else {
        printf("Лес-кольцо. Переходов через край: %d\n",
               map->edge_transitions);
        printf("WASD — движение, Q — выход с портала P\n");
    }
}

void fill_empty_map(GameMap *map) {
    for (int row = 0; row < map->rows; ++row) {
        for (int col = 0; col < map->cols; ++col) {
            int index = map_index(map, row, col);

            if (map->mode == DUNGEON_MODE &&
                (row == 0 || row == map->rows - 1 ||
                 col == 0 || col == map->cols - 1)) {
                map->cells[index] = '#';
            } else {
                map->cells[index] = '.';
            }
        }
    }
}
void place_random_obstacles(GameMap *map) {
    int percent = FOREST_MIN_OBSTACLE_PERCENT +
                  rand() % (FOREST_MAX_OBSTACLE_PERCENT -
                            FOREST_MIN_OBSTACLE_PERCENT + 1);

    for (int row = 1; row < map->rows - 1; ++row) {
        for (int col = 1; col < map->cols - 1; ++col) {
            int random_value = rand() % 100;

            if (random_value < percent) {
                if (rand() % 2 == 0) {
                    map->cells[map_index(map, row, col)] = 'T';
                } else {
                    map->cells[map_index(map, row, col)] = 'R';
                }
            }
        }
    }
}
int is_empty_cell(const GameMap *map, int row, int col) {
    char cell = map->cells[map_index(map, row, col)];
    return cell == '.';
}

void place_player(GameMap *map) {
    int row;
    int col;

    do {
        row = 1 + rand() % (map->rows - 2);
        col = 1 + rand() % (map->cols - 2);
    } while (!is_empty_cell(map, row, col));

    map->player_row = row;
    map->player_col = col;
}
void place_dungeon_stairs(GameMap *map) {
    int row;
    int col;

    do {
        row = 1 + rand() % (map->rows - 2);
        col = 1 + rand() % (map->cols - 2);
    } while (!is_empty_cell(map, row, col) ||
             (row == map->player_row && col == map->player_col));

    map->cells[map_index(map, row, col)] = 'D';
//Лестница U располагается рядом с точкой старта. Сначала пробуем клетку справа, затем остальные соседние клетки.
    int directions[4][2] = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0}
    };

    for (int i = 0; i < 4; ++i) {
        int stair_row = map->player_row + directions[i][0];
        int stair_col = map->player_col + directions[i][1];

        if (stair_row > 0 && stair_row < map->rows - 1 &&
            stair_col > 0 && stair_col < map->cols - 1 &&
            is_empty_cell(map, stair_row, stair_col)) {
            map->cells[map_index(map, stair_row, stair_col)] = 'U';
            return;
        }
    }
}
void place_forest_portal(GameMap *map) {
    int row;
    int col;

    do {
        row = rand() % map->rows;
        col = rand() % map->cols;
    } while (!is_empty_cell(map, row, col) ||
             (row == map->player_row && col == map->player_col));

    map->cells[map_index(map, row, col)] = 'P';
}
int create_map(GameMap *map, int mode, int depth) {
    map->rows = MAP_MIN_SIZE +
                rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    map->cols = MAP_MIN_SIZE +
                rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);

    map->mode = mode;
    map->depth = depth;
    map->edge_transitions = 0;

    map->cells = malloc((size_t)map->rows *
                        (size_t)map->cols *
                        sizeof(char));

    if (map->cells == NULL) {
        printf("[Ошибка] Не удалось выделить память под карту.\n");
        return 0;
    }
    fill_empty_map(map);
    place_random_obstacles(map);
    place_player(map);

    if (mode == DUNGEON_MODE) {
        place_dungeon_stairs(map);
    }

    return 1;
}
int try_move(GameMap *map, char command) {
    int new_row = map->player_row;
    int new_col = map->player_col;

    if (command == 'w' || command == 'W') {
        new_row--;
    } else if (command == 's' || command == 'S') {
        new_row++;
    } else if (command == 'a' || command == 'A') {
        new_col--;
    } else if (command == 'd' || command == 'D') {
        new_col++;
    } else {
        printf("[Ошибка] Используйте W, A, S, D или Q.\n");
        return 0;
    }

    if (map->mode == DUNGEON_MODE) {
        if (new_row < 0 || new_row >= map->rows ||
            new_col < 0 || new_col >= map->cols) {
            printf("За стену пройти нельзя.\n");
            return 0;
        }
    } else {
        int crossed_edge = 0;

        if (new_row < 0) {
            new_row = map->rows - 1;
            crossed_edge = 1;
        } else if (new_row >= map->rows) {
            new_row = 0;
            crossed_edge = 1;
        }

        if (new_col < 0) {
            new_col = map->cols - 1;
            crossed_edge = 1;
        } else if (new_col >= map->cols) {
            new_col = 0;
            crossed_edge = 1;
        }
        if (crossed_edge) {
            map->edge_transitions++;

            printf("Вы перешли через край карты.\n");

            if (map->edge_transitions >= 3) {
                int portal_exists = 0;

                for (int row = 0; row < map->rows; ++row) {
                    for (int col = 0; col < map->cols; ++col) {
                        if (map->cells[map_index(map, row, col)] == 'P') {
                            portal_exists = 1;
                        }
                    }
                }
                if (!portal_exists) {
                    place_forest_portal(map);
                    printf("На карте появился портал P!\n");
                }
            }
        }
    }
char destination = map->cells[map_index(map, new_row, new_col)];
    if (is_blocked(destination)) {
        printf("Пройти нельзя: впереди препятствие.\n");
        return 0;
    }
    map->player_row = new_row;
    map->player_col = new_col;
    return 1;
}
void run_map_game(void) {
    GameMap map = {0};
    int mode;
    int depth = 1;
    char command;
    printf("\n--- РЕЖИМ КАРТЫ ---\n");
    printf("[1] Подземелье\n");
    printf("[2] Лес-кольцо\n");
    mode = get_valid_int(
        DUNGEON_MODE,
        FOREST_MODE,
        "Выберите режим: "
    );
    srand((unsigned int)time(NULL));
    if (!create_map(&map, mode, depth)) {
        return;
    }

    while (1) {
        print_map(&map);
        printf("Ваш ход: ");

        if (scanf(" %c", &command) != 1) {
            clear_input_buffer();
            printf("[Ошибка] Не удалось прочитать команду.\n");
            continue;
}
        if (command == 'q' || command == 'Q') {
            char current_cell =
                map.cells[map_index(&map,
                                    map.player_row,
                                    map.player_col)];

            if (map.mode == DUNGEON_MODE && current_cell == 'U') {
                printf("Вы поднялись по лестнице и вернулись домой.\n");
                break;
}
            if (map.mode == FOREST_MODE && current_cell == 'P') {
                printf("Вы вошли в портал и вернулись домой.\n");
                break;
}
            printf("Сбежать домой можно только с клетки U или P.\n");
            continue;
}

        if (!try_move(&map, command)) {
            continue;
}
        char current_cell =
            map.cells[map_index(&map,
                                map.player_row,
                                map.player_col)];
        if (map.mode == DUNGEON_MODE && current_cell == 'D') {
            printf("Вы нашли лестницу вниз. Загружается новый этаж...\n");

            depth++;
            free_map(&map);

            if (!create_map(&map, DUNGEON_MODE, depth)) {
                return;
            }
}
        if (map.mode == FOREST_MODE && current_cell == 'P') {
            printf("Вы вошли в портал. Генерируется новая карта...\n");

            free_map(&map);

            if (!create_map(&map, FOREST_MODE, 1)) {
                return;
            }
        }
    }

    free_map(&map);
}
//точка входа
int main(void) {
    int current_day = 1;
    int current_hour = 8;
    char farmer_name[MAX_FARMER_NAME_LEN];
    char item_names[MAX_ITEMS][MAX_NAME_LEN];

   //Начальный рюкзак игрока: 2 Дерева (1), 1 Камень (2), 1 Семена (3), 1 Железо (4), 1 Ткань (5)
    int inventory[INVENTORY_SIZE] = {
        ITEM_WOOD, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS,
        ITEM_IRON, ITEM_CLOTH, EMPTY_SLOT, EMPTY_SLOT,
        EMPTY_SLOT, EMPTY_SLOT
    };

    //1. Имя фермера 
    init_farmer_name(farmer_name);

    /* 2. Загрузка каталога предметов из items.txt с защитой */
    load_item_catalog(item_names);

    //Главный цикл программы 
    while (1) {
        print_menu(farmer_name);
        int choice = get_valid_int(0, 9, "Выберите пункт (0-9): ");

        switch (choice) {
            case 0:
                printf("\nСохранение завершено. До свидания, %s!\n", farmer_name);
                return 0;
            case 1:
                show_time(current_day, current_hour);
                break;
            case 2:
                pass_time(&current_day, &current_hour);
                break;
            case 3:
                show_inventory(inventory, item_names);
                break;
            case 4:
                put_item(inventory, item_names);
                break;
            case 5:
                drop_item(inventory, item_names);
                break;
            case 6:
                audit_resources(inventory);
                break;
            case 7:
                search_item_by_name(inventory, item_names);
                break;
            case 8:
                save_diary(farmer_name, current_day, current_hour, inventory, item_names);
                break;
            case 9:
                check_crafting_recipes(inventory, item_names);
                break;
            default:
                break;
        }
    }

    return 0;
}
