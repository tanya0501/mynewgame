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
