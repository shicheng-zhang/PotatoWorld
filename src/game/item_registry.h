#ifndef game_item_registry_h
#define game_item_registry_h

#include <stdbool.h>
#include "../core/math3D.h"
#include "block.h"

#define ITEM_TYPE_MAX 256

typedef struct {
    const char* name;
    int block_id;
    float colour_r, colour_g, colour_b;
    int max_stack_size;
    int max_damage;
    tool_type_t tool_type;
    int tool_tier;
    float attack_damage;
    float attack_speed;
    int food_value;
    float saturation;
    bool is_food;
    bool placeable;
} item_type;

void item_registry_init(void);
int item_type_register(const char* name, int block_id, float r, float g, float b,
                          int max_stack, int max_damage, tool_type_t tool, int tier,
                          float atk_dmg, float atk_spd, int food, float saturation,
                          bool is_food, bool placeable);
const item_type* item_type_get(int item_id);
int item_type_count(void);

#endif