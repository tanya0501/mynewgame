#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "utils.h"

const Food FOOD_RATION = {"Еда", 30, 15};

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

void load_item_catalog(char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE *file = fopen(ITEMS_FILE, "r");
    if (!file) {
        printf("[Предупреждение] Файл '%s' не найден! Загружен базовый каталог.\n", ITEMS_FILE);
        set_default_item_catalog(item_names);
        return;
    }

    set_default_item_catalog(item_names);

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

void free_player(Player **player) {
    if (player != NULL && *player != NULL) {
        printf("[Память] Освобождается память персонажа по адресу %p.\n", (void *)*player);
        free(*player);
        *player = NULL;
    }
}

Player *create_player(void) {
    Player *player = malloc(sizeof(*player));
    if (player == NULL) {
        printf("[Ошибка] Не удалось выделить память под персонажа!\n");
        return NULL;
    }

    printf("Введите имя вашего фермера: ");
    if (fgets(player->name, sizeof(player->name), stdin)) {
        remove_newline(player->name);
        if (strlen(player->name) == 0) {
            strncpy(player->name, "Безымянный фермер", MAX_FARMER_NAME_LEN - 1);
        }
    } else {
        strncpy(player->name, "Безымянный фермер", MAX_FARMER_NAME_LEN - 1);
    }
    player->name[MAX_FARMER_NAME_LEN - 1] = '\0';

    player->max_hp = START_HP;
    player->hp = player->max_hp;
    player->max_stamina = START_STAMINA;
    player->stamina = player->max_stamina;
    player->hunger = MAX_HUNGER;
    player->gold = 0;

    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        player->inventory[i] = EMPTY_SLOT;
    }
    player->inventory[0] = ITEM_WOOD;
    player->inventory[1] = ITEM_WOOD;
    player->inventory[2] = ITEM_STONE;
    player->inventory[3] = ITEM_SEEDS;
    player->inventory[4] = ITEM_IRON;
    player->inventory[5] = ITEM_CLOTH;
    player->inventory[6] = ITEM_FOOD;
    player->inventory[7] = ITEM_FOOD;

    printf("[Успешно] Создан персонаж '%s' по адресу %p.\n", player->name, (void *)player);
    return player;
}

int check_player_exists(const Player *player) {
    if (player == NULL) {
        printf("[Предупреждение] Сначала создайте персонажа (Пункт [1])!\n");
        return 0;
    }
    return 1;
}

void show_player_card(const Player *player, int day, int hour, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    printf("\n=== КАРТОЧКА ПЕРСОНАЖА ===\n");
    printf("Имя: %s\n", player->name);
    printf("Здоровье: %d/%d HP\n", player->hp, player->max_hp);
    printf("Выносливость: %d/%d\n", player->stamina, player->max_stamina);
    printf("Сытость: %d/%d\n", player->hunger, MAX_HUNGER);
    printf("Накопленное золото: %d\n", player->gold);
    printf("Дата: День %d, %02d:00\n", day, hour);

    printf("\nСодержимое рюкзака:\n");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = player->inventory[i];
        const char *name = (id >= 0 && id < MAX_ITEMS) ? item_names[id] : "Неизвестно";
        printf("  Слот %d: [%d] %s\n", i, id, name);
    }
}

int eat_food(Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    (void)item_names;
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (player->inventory[i] == ITEM_FOOD) {
            player->inventory[i] = EMPTY_SLOT;
            player->hunger += FOOD_RATION.hunger_restore;
            player->stamina += FOOD_RATION.stamina_restore;

            if (player->hunger > MAX_HUNGER) player->hunger = MAX_HUNGER;
            if (player->stamina > player->max_stamina) player->stamina = player->max_stamina;

            printf("[Питание] Съеден предмет '%s'! Сытость: %d/%d, Выносливость: %d/%d.\n",
                   FOOD_RATION.name, player->hunger, MAX_HUNGER, player->stamina, player->max_stamina);
            return 1;
        }
    }
    printf("[Питание] В вашем рюкзаке нет еды!\n");
    return 0;
}

void show_inventory(const Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    printf("\n--- СОДЕРЖИМОЕ ИНВЕНТАРЯ ---\n");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = player->inventory[i];
        const char *name = (id >= 0 && id < MAX_ITEMS) ? item_names[id] : "Неизвестно";
        printf("Слот %d: [%d] - %s\n", i, id, name);
    }
}

void put_item(Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int slot = get_valid_int(0, INVENTORY_SIZE - 1, "Введите индекс слота (0-9): ");
    int item_id = get_valid_int(0, MAX_ITEMS - 1, "Введите ID предмета (0-9): ");

    player->inventory[slot] = item_id;
    printf("Предмет \"%s\" (ID %d) успешно помещён в слот %d.\n", item_names[item_id], item_id, slot);
}

void drop_item(Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int slot = get_valid_int(0, INVENTORY_SIZE - 1, "Введите индекс слота для удаления (0-9): ");
    int old_id = player->inventory[slot];

    player->inventory[slot] = EMPTY_SLOT;
    printf("Слот %d очищен. Предмет \"%s\" (ID %d) выброшен.\n", slot, item_names[old_id], old_id);
}

void audit_resources(const Player *player) {
    int target_id = get_valid_int(0, MAX_ITEMS - 1, "Введите ID предмета для ревизии (0-9): ");
    int count = 0;

    printf("Позиции слотов: ");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (player->inventory[i] == target_id) {
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

void search_item_by_name(const Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
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
        if (player->inventory[i] == found_id) {
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

void save_diary(const Player *player, int day, int hour, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE *diary = fopen(DIARY_FILE, "a");
    if (!diary) {
        printf("[Ошибка] Не удалось открыть или создать файл дневника '%s'!\n", DIARY_FILE);
        return;
    }

    printf("\n--- СОХРАНЕНИЕ В ДНЕВНИК ФЕРМЕРА ---\n");
    printf("Фермер: %s | День %d | Время %02d:00\n", player->name, day, hour);
    printf("Содержимое рюкзака:\n");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = player->inventory[i];
        printf("  Слот %d: [%d] - %s\n", i, id, item_names[id]);
    }

    fprintf(diary, "--- ЗАПИСЬ В ДНЕВНИКЕ ---\n");
    fprintf(diary, "Фермер: %s | День %d | Время %02d:00\n", player->name, day, hour);
    fprintf(diary, "Содержимое рюкзака:\n");
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = player->inventory[i];
        fprintf(diary, "  Слот %d: [%d] - %s\n", i, id, item_names[id]);
    }
    fprintf(diary, "-------------------------\n\n");

    fclose(diary);
    printf("[Успешно] Состояние зафиксировано на экране и записано в '%s'!\n", DIARY_FILE);
}

void check_crafting_recipes(const Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
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

    int available_resources[MAX_ITEMS] = {0};
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        int id = player->inventory[i];
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

        char *colon_ptr = strchr(line, ':');
        if (!colon_ptr) continue;

        *colon_ptr = '\0';
        char recipe_name[MAX_NAME_LEN];
        strncpy(recipe_name, line, MAX_NAME_LEN - 1);
        recipe_name[MAX_NAME_LEN - 1] = '\0';

        int name_len = strlen(recipe_name);
        while (name_len > 0 && recipe_name[name_len - 1] == ' ') {
            recipe_name[name_len - 1] = '\0';
            name_len--;
        }

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

        int missing_len = strlen(missing_desc);
        if (missing_len >= 2 && missing_desc[missing_len - 2] == ',') {
            missing_desc[missing_len - 2] = '\0';
        }

        char result_line[BUFFER_SIZE * 2];
        if (can_craft) {
            snprintf(result_line, sizeof(result_line), "%s: [ДОСТУПНО]", recipe_name);
        } else {
            snprintf(result_line, sizeof(result_line), "%s: [НЕ ХВАТАЕТ РЕСУРСОВ] (Недостает: %s)", recipe_name, missing_desc);
        }

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
