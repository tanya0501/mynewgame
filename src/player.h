#ifndef PLAYER_H
#define PLAYER_H

#define INVENTORY_SIZE      10
#define MAX_ITEMS           10
#define MAX_NAME_LEN        32
#define MAX_FARMER_NAME_LEN 64

#define ITEMS_FILE          "items.txt"
#define DIARY_FILE          "diary.txt"
#define INPUT_FILE          "input.txt"
#define OUTPUT_FILE         "output.txt"

#define EMPTY_SLOT 0
#define ITEM_WOOD  1
#define ITEM_STONE 2
#define ITEM_SEEDS 3
#define ITEM_IRON  4
#define ITEM_CLOTH 5
#define ITEM_FOOD  6

#define START_HP 100
#define START_STAMINA 50
#define MAX_HUNGER 100
#define HUNGER_DANGER_LIMIT 20

typedef struct {
    char name[MAX_NAME_LEN];
    int hunger_restore;
    int stamina_restore;
} Food;

typedef struct {
    char name[MAX_FARMER_NAME_LEN];
    int hp;
    int max_hp;
    int stamina;
    int max_stamina;
    int hunger;
    int gold;
    int inventory[INVENTORY_SIZE];
} Player;

extern const Food FOOD_RATION;

void set_default_item_catalog(char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void load_item_catalog(char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void free_player(Player **player);
Player *create_player(void);
int check_player_exists(const Player *player);
void show_player_card(const Player *player, int day, int hour, char item_names[MAX_ITEMS][MAX_NAME_LEN]);
int eat_food(Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void show_inventory(const Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void put_item(Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void drop_item(Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void audit_resources(const Player *player);
void search_item_by_name(const Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void save_diary(const Player *player, int day, int hour, char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void check_crafting_recipes(const Player *player, char item_names[MAX_ITEMS][MAX_NAME_LEN]);

#endif /* PLAYER_H */
