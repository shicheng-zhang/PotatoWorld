#ifndef game_player_h
#define game_player_h

#include <stdbool.h>
#include "../core/math3D.h"
#include "block.h"

#define INVENTORY_HOTBAR_SIZE 9
#define INVENTORY_MAIN_SIZE 27
#define INVENTORY_CRAFTING_GRID_SIZE 4
#define INVENTORY_CRAFTING_TABLE_SIZE 9
#define INVENTORY_TOTAL_SLOTS (INVENTORY_HOTBAR_SIZE + INVENTORY_MAIN_SIZE + INVENTORY_CRAFTING_GRID_SIZE + 1)
#define MAX_STACK_SIZE 64

typedef enum {
    GAME_MODE_SURVIVAL = 0,
    GAME_MODE_CREATIVE = 1,
    GAME_MODE_ADVENTURE = 2,
    GAME_MODE_SPECTATOR = 3,
} game_mode_t;

typedef struct {
    int item_id;
    int count;
    int damage;
} item_stack_t;

typedef struct {
    item_stack_t slots[INVENTORY_TOTAL_SLOTS];
    int selected_hotbar_slot;
    bool inventory_open;
    bool crafting_table_open;
    int crafting_grid_type;
} player_inventory_t;

typedef struct {
    float health;
    float max_health;
    float hunger;
    float max_hunger;
    float saturation;
    float exhaustion;
    int experience_level;
    int experience_progress;
    int experience_total;
    game_mode_t game_mode;
    bool flying;
    bool no_clip;
    float fly_speed;
    vector3 spawn_position;
    bool spawn_set;
    int hurt_time;
    int death_time;
    bool is_dead;
} player_stats_t;

typedef struct {
    player_stats_t stats;
    player_inventory_t inventory;
    vector3 eye_position;
    float eye_height;
    bool on_ground;
    int air_supply;
    int max_air;
    int fire_ticks;
    int freeze_ticks;
} player_t;

extern player_t g_player;

void player_init(void);
void player_update(float delta_time);
void player_take_damage(float amount, int damage_type);
void player_heal(float amount);
void player_add_exhaustion(float amount);
void player_add_experience(int amount);
void player_set_game_mode(game_mode_t mode);
void player_toggle_creative(void);
void player_respawn(void);
void player_set_spawn(vector3 pos);

bool inventory_add_item(int item_id, int count, int damage);
bool inventory_remove_item(int item_id, int count);
int inventory_get_count(int item_id);
item_stack_t* inventory_get_slot(int slot_index);
void inventory_swap_slots(int from, int to);
void inventory_move_to_hotbar(int from_slot);
void inventory_select_hotbar_slot(int slot);
void inventory_open_close(void);
void inventory_crafting_table_open_close(void);
void inventory_crafting_grid_craft(void);
void inventory_clear_crafting_grid(void);

bool player_can_break_block(int block_id, tool_type_t tool, int tool_tier);
float player_get_break_speed(int block_id, tool_type_t tool, int tool_tier);
int player_get_item_in_hand(void);
void player_swing_hand(void);

#define INV_SLOT_HOTBAR_START 0
#define INV_SLOT_HOTBAR_END 8
#define INV_SLOT_MAIN_START 9
#define INV_SLOT_MAIN_END 35
#define INV_SLOT_CRAFTING_GRID_START 36
#define INV_SLOT_CRAFTING_GRID_END 39
#define INV_SLOT_CRAFTING_RESULT 40
#define INV_SLOT_CRAFTING_TABLE_START 41
#define INV_SLOT_CRAFTING_TABLE_END 49
#define INV_SLOT_CRAFTING_TABLE_RESULT 50

#endif