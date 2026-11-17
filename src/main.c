#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "player.h"
#include "map.h"

#define NIGHT_HOUR 2
#define MORNING_HOUR 8
#define HOURS_IN_DAY 24
#define HUNGER_LOSS_PER_HOUR 5

static void show_time(int day, int hour) {
    printf("Текущее время: День %d, %02d:00\n", day, hour);
}

static void print_menu(const Player *player) {
    printf("\n==================================================\n");
    printf(" Ферма игрока: %s\n", player == NULL ? "[Персонаж не создан]" : player->name);
    printf("==================================================\n");
    printf(" [0] Выход\n");
    printf(" [1] Создать / пересоздать персонажа\n");
    printf(" [2] Карточка персонажа\n");
    
    printf(" [3] Посмотреть инвентарь\n");
    printf(" [4] Положить предмет в слот\n");
    printf(" [5] Выбросить предмет\n");
    printf(" [6] Ревизия ресурсов\n");
    printf(" [7] Поиск предмета в рюкзаке по названию\n");
    printf(" [8] Записать состояние в дневник фермера (%s)\n", DIARY_FILE);
    printf(" [9] Проверить рецепты крафта (%s -> %s)\n", INPUT_FILE, OUTPUT_FILE);
    printf(" [10] Исследовать лес\n");
    printf(" [11] Время и отдых\n");
    printf("--------------------------------------------------\n");
}

static void time_and_rest_menu(int *day, int *hour, Player *player) {
    int choice;
    do {
        printf("\n--- РЕЖИМ ВРЕМЕНИ И ОТДЫХА ---\n");
        printf(" 1 — Посмотреть на часы\n");
        printf(" 2 — Пропустить время (Поработать)\n");
        printf(" 3 — Отдых (Лечь спать)\n");
        printf(" 0 — Назад в главное меню\n");
        choice = get_valid_int(0, 3, "Выберите действие (0-3): ");

        if (choice == 1) {
            show_time(*day, *hour);
        } else if (choice == 2) {
            int until_night = (*hour < NIGHT_HOUR) ? NIGHT_HOUR - *hour : 24 - *hour + NIGHT_HOUR;
            printf("Вы можете пропустить до %d часов (до ночи 02:00).\n", until_night);
            int hours = get_valid_int(1, until_night, "Сколько часов пропустить? ");
            *hour = (*hour + hours) % HOURS_IN_DAY;

            if (player != NULL) {
                player->hunger -= hours * HUNGER_LOSS_PER_HOUR;
                if (player->hunger < 0) player->hunger = 0;
            }
            printf("Время промотано. ");
            show_time(*day, *hour);
        } else if (choice == 3) {
            (*day)++;
            *hour = MORNING_HOUR;
            if (player != NULL) {
                player->hp = player->max_hp;
                player->stamina = player->max_stamina;
                player->hunger = MAX_HUNGER;
            }
            printf("Вы хорошо выспались! Наступило утро нового дня.\n");
            show_time(*day, *hour);
        }
    } while (choice != 0);
}

static void run_map_game(Player *player, int *day, int *hour, char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    if (!check_player_exists(player)) return;

    if (player->stamina <= 0 || (*hour >= NIGHT_HOUR && *hour < MORNING_HOUR)) {
        printf("[Поход невозможен] Вы слишком устали или на улице ночь (02:00 - 08:00)!\n");
        return;
    }

    GameMap map = {0};
    char command;
    int distance = 1;
    int steps = 0;

    srand((unsigned int)time(NULL));
    if (!create_map(&map, distance)) return;

    for (;;) {
        print_map(&map, player, *day, *hour);
        printf("Ваш ход: ");

        if (scanf(" %c", &command) != 1) {
            clear_input_buffer();
            printf("[Ошибка] Не удалось прочитать команду.\n");
            continue;
        }

        if (command == 'e' || command == 'E') {
            eat_food(player, item_names);
            continue;
        }

        if (command == 'q' || command == 'Q') {
            char current_cell = map.cells[map_index(&map, map.player_row, map.player_col)];
            if (current_cell == '^') {
                printf("Вы безопасно вернулись домой! Заработанное золото сохранено.\n");
                break;
            }
            printf("Вернуться домой можно только с клетки лагеря ^.\n");
            continue;
        }

        if (!try_move(&map, player, command)) continue;

        steps++;
        if (steps % 5 == 0) {
            player->hunger -= HUNGER_LOSS_PER_FIVE_STEPS;
            if (player->hunger < 0) player->hunger = 0;
            printf("[Сытость] За каждые 5 шагов сытость упала на %d.\n", HUNGER_LOSS_PER_FIVE_STEPS);
        }

        char current_cell = map.cells[map_index(&map, map.player_row, map.player_col)];
        if (current_cell == 'P') {
            player->stamina -= NEW_MAP_STAMINA_COST;
            *hour = (*hour + NEW_MAP_HOUR_COST) % HOURS_IN_DAY;
            distance++;
            int reward = GOLD_REWARD_MULTIPLIER * distance;
            player->gold += reward;

            printf("\n[ПОРТАЛ] Вы совершили переход на Этаж %d! Награда: +%d золота.\n", distance, reward);
            free_map(&map);

            if (player->stamina <= 0 || (*hour >= NIGHT_HOUR && *hour < MORNING_HOUR)) {
                printf("[ОБМОРОК/НОЧЬ] У вас не осталось сил или наступила ночь!\n");
                player->gold /= 2;
                return;
            }

            if (!create_map(&map, distance)) {
                player->gold /= 2;
                return;
            }
        }

        if (player->stamina <= 0 || player->hp <= 0 || (*hour >= NIGHT_HOUR && *hour < MORNING_HOUR)) {
            printf("\n[ЭВАКУАЦИЯ] Персонаж потерял сознание! Спасатели забрали 50%% золота и вернули вас домой.\n");
            player->gold /= 2;
            break;
        }
    }

    free_map(&map);
}

int main(void) {
    int current_day = 1;
    int current_hour = MORNING_HOUR;
    Player *player = NULL;
    char item_names[MAX_ITEMS][MAX_NAME_LEN];

    load_item_catalog(item_names);

    while (1) {
        print_menu(player);
        int choice = get_valid_int(0, 11, "Выберите пункт (0-11): ");

        switch (choice) {
            case 0:
                free_player(&player);
                printf("\nСохранение завершено. До свидания!\n");
                return 0;

            case 1:
                if (player != NULL) {
                    int confirm = get_valid_int(0, 1, "Персонаж уже существует! Пересоздать? (1 — Да, 0 — Нет): ");
                    if (confirm == 0) break;
                    free_player(&player);
                }
                player = create_player();
                break;

            case 2:
                if (check_player_exists(player)) {
                    show_player_card(player, current_day, current_hour, item_names);  
                }
                break;

            case 3:
                if (check_player_exists(player)) show_inventory(player, item_names);
                break;

            case 4:
                if (check_player_exists(player)) put_item(player, item_names);
                break;

            case 5:
                if (check_player_exists(player)) drop_item(player, item_names);
                break;

            case 6:
                if (check_player_exists(player)) audit_resources(player);
                break;

            case 7:
                if (check_player_exists(player)) search_item_by_name(player, item_names);
                break;

            case 8:
                if (check_player_exists(player)) save_diary(player, current_day, current_hour, item_names);
                break;

            case 9:
                if (check_player_exists(player)) check_crafting_recipes(player, item_names);
                break;

            case 10:
                if (check_player_exists(player)) run_map_game(player, &current_day, &current_hour, item_names);
                break;

            default:
                break;
            
            case 11:
				time_and_rest_menu(&current_day, &current_hour, player);
				break;
        }
    }

    return 0;
}
