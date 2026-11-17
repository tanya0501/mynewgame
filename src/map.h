#ifndef MAP_H
#define MAP_H

#include "player.h"

#define MAP_MIN_SIZE 5
#define MAP_MAX_SIZE 30
#define FOREST_MIN_OBSTACLE_PERCENT 15
#define FOREST_MAX_OBSTACLE_PERCENT 30

#define STEP_STAMINA_COST 1
#define NEW_MAP_STAMINA_COST 5
#define NEW_MAP_HOUR_COST 1
#define GOLD_REWARD_MULTIPLIER 10
#define HUNGER_LOSS_PER_FIVE_STEPS 5
#define LOW_HUNGER_HP_LOSS 1

typedef struct {
    int rows;
    int cols;
    char *cells;
    int player_row;
    int player_col;
    int distance;
    int edge_transitions;
} GameMap;

void free_map(GameMap *map);
int map_index(const GameMap *map, int row, int col);
int is_blocked(char cell);
int is_empty_cell(const GameMap *map, int row, int col);
void print_map(const GameMap *map, const Player *player, int day, int hour);
void fill_empty_map(GameMap *map);
void place_random_obstacles(GameMap *map);
void place_player_on_map(GameMap *map);
void place_camp(GameMap *map);
void place_portal(GameMap *map);
int portal_exists(const GameMap *map);
int create_map(GameMap *map, int distance);
int try_move(GameMap *map, Player *player, char command);

#endif /* MAP_H */
