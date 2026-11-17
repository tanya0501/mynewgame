#include <stdio.h>
#include <stdlib.h>
#include "map.h"

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

int is_empty_cell(const GameMap *map, int row, int col) {
    return map->cells[map_index(map, row, col)] == '.';
}

void print_map(const GameMap *map, const Player *player, int day, int hour) {
    printf("\n--- ЛЕС: Участок %d ---\n", map->distance);
    printf("[HUD] HP: %d/%d | Выносливость: %d/%d | Сытость: %d/%d | Золото: %d | Этаж: %d | Время: %d, %02d:00\n",
           player->hp, player->max_hp, player->stamina, player->max_stamina,
           player->hunger, MAX_HUNGER, player->gold, map->distance, day, hour);

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
    printf("W/A/S/D — движение, E — съехать паёк, Q — вернуться домой (только с клетки ^)\n");
}

void fill_empty_map(GameMap *map) {
    for (int row = 0; row < map->rows; ++row) {
        for (int col = 0; col < map->cols; ++col) {
            map->cells[map_index(map, row, col)] = '.';
        }
    }
}

void place_random_obstacles(GameMap *map) {
    int percent = FOREST_MIN_OBSTACLE_PERCENT +
        rand() % (FOREST_MAX_OBSTACLE_PERCENT - FOREST_MIN_OBSTACLE_PERCENT + 1);
    for (int row = 0; row < map->rows; ++row) {
        for (int col = 0; col < map->cols; ++col) {
            if (rand() % 100 < percent) {
                map->cells[map_index(map, row, col)] = (rand() % 2 == 0) ? 'T' : 'R';
            }
        }
    }
}

void place_player_on_map(GameMap *map) {
    int row, col;
    do {
        row = rand() % map->rows;
        col = rand() % map->cols;
    } while (!is_empty_cell(map, row, col));

    map->player_row = row;
    map->player_col = col;
}

void place_camp(GameMap *map) {
    int row, col;
    do {
        row = rand() % map->rows;
        col = rand() % map->cols;
    } while (!is_empty_cell(map, row, col) ||
             (row == map->player_row && col == map->player_col));

    map->cells[map_index(map, row, col)] = '^';
}

void place_portal(GameMap *map) {
    int row, col;
    do {
        row = rand() % map->rows;
        col = rand() % map->cols;
    } while (!is_empty_cell(map, row, col) ||
             (row == map->player_row && col == map->player_col));

    map->cells[map_index(map, row, col)] = 'P';
}

int portal_exists(const GameMap *map) {
    for (int row = 0; row < map->rows; ++row) {
        for (int col = 0; col < map->cols; ++col) {
            if (map->cells[map_index(map, row, col)] == 'P') return 1;
        }
    }
    return 0;
}

int create_map(GameMap *map, int distance) {
    map->rows = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    map->cols = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    map->distance = distance;
    map->edge_transitions = 0;

    map->cells = malloc((size_t)map->rows * (size_t)map->cols * sizeof(char));
    if (map->cells == NULL) {
        printf("[Ошибка] Не удалось выделить память под карту.\n");
        return 0;
    }
    fill_empty_map(map);
    place_random_obstacles(map);
    place_player_on_map(map);
    place_camp(map);

    return 1;
}

int try_move(GameMap *map, Player *player, char command) {
    int new_row = map->player_row;
    int new_col = map->player_col;
    int crossed_edge = 0;

    if (command == 'w' || command == 'W') new_row--;
    else if (command == 's' || command == 'S') new_row++;
    else if (command == 'a' || command == 'A') new_col--;
    else if (command == 'd' || command == 'D') new_col++;
    else {
        printf("[Ошибка] Неизвестная команда. Используйте W, A, S, D, E или Q.\n");
        return 0;
    }

    if (new_row < 0) { new_row = map->rows - 1; crossed_edge = 1; }
    else if (new_row >= map->rows) { new_row = 0; crossed_edge = 1; }

    if (new_col < 0) { new_col = map->cols - 1; crossed_edge = 1; }
    else if (new_col >= map->cols) { new_col = 0; crossed_edge = 1; }

    char destination = map->cells[map_index(map, new_row, new_col)];
    if (is_blocked(destination)) {
        printf("Пройти нельзя: впереди препятствие.\n");
        return 0;
    }

    map->player_row = new_row;
    map->player_col = new_col;

    player->stamina -= STEP_STAMINA_COST;

    if (player->hunger < HUNGER_DANGER_LIMIT) {
        player->hp -= LOW_HUNGER_HP_LOSS;
        if (player->hp < 0) player->hp = 0;
        printf("[Голодание] От сильного голода потеряно %d HP!\n", LOW_HUNGER_HP_LOSS);
    }

    if (crossed_edge) {
        map->edge_transitions++;
        printf("Вы перешли через край карты (%d).\n", map->edge_transitions);

        if (map->edge_transitions >= 3 && !portal_exists(map)) {
            place_portal(map);
            printf("На карте появился портал P!\n");
        }
    }
    return 1;
}
