#include "player.h"
#include "../mpe_engine.h"
#include "block.h"
#include "item_registry.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

player_t g_player;

#define DAMAGE_GENERIC 0
#define DAMAGE_FALL 1
#define DAMAGE_FIRE 2
#define DAMAGE_DROWN 3
#define DAMAGE_SUFFOCATE 4

static const float BASE_HEALTH = 20.0f;
static const float BASE_HUNGER = 20.0f;
static const float BASE_SATURATION = 5.0f;
static const float WALK_EXHAUSTION = 0.01f;
static const float SPRINT_EXHAUSTION = 0.1f;
static const float JUMP_EXHAUSTION = 0.05f;
static const float BREAK_BLOCK_EXHAUSTION = 0.025f;
static const float ATTACK_EXHAUSTION = 0.3f;
static const float REGEN_HUNGER_THRESHOLD = 18.0f;
static const float REGEN_RATE = 1.0f;
static const float STARVATION_DAMAGE = 1.0f;

void player_init(void) {
    (void)SPRINT_EXHAUSTION; (void)JUMP_EXHAUSTION; (void)BREAK_BLOCK_EXHAUSTION; (void)ATTACK_EXHAUSTION;
    memset(&g_player, 0, sizeof(player_t));
    
    g_player.stats.health = BASE_HEALTH;
    g_player.stats.max_health = BASE_HEALTH;
    g_player.stats.hunger = BASE_HUNGER;
    g_player.stats.max_hunger = BASE_HUNGER;
    g_player.stats.saturation = BASE_SATURATION;
    g_player.stats.exhaustion = 0.0f;
    g_player.stats.experience_level = 0;
    g_player.stats.experience_progress = 0;
    g_player.stats.experience_total = 0;
    g_player.stats.game_mode = GAME_MODE_SURVIVAL;
    g_player.stats.flying = false;
    g_player.stats.no_clip = false;
    g_player.stats.fly_speed = 25.0f;
    g_player.stats.hurt_time = 0;
    g_player.stats.death_time = 0;
    g_player.stats.is_dead = false;
    
    g_player.eye_height = 1.62f;
    g_player.air_supply = 300;
    g_player.max_air = 300;
    g_player.fire_ticks = 0;
    g_player.freeze_ticks = 0;
    
    g_player.inventory.selected_hotbar_slot = 0;
    g_player.inventory.inventory_open = false;
    g_player.inventory.crafting_table_open = false;
    g_player.inventory.crafting_grid_type = 0;
    
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; i++) {
        g_player.inventory.slots[i].item_id = ITEM_AIR;
        g_player.inventory.slots[i].count = 0;
        g_player.inventory.slots[i].damage = 0;
    }
    
    inventory_add_item(ITEM_STONE_PICKAXE, 1, 0);
    inventory_add_item(ITEM_STONE_AXE, 1, 0);
    inventory_add_item(ITEM_STONE_SHOVEL, 1, 0);
    inventory_add_item(ITEM_COBBLESTONE, 64, 0);
    inventory_add_item(ITEM_OAK_LOG, 16, 0);
    inventory_add_item(ITEM_BREAD, 10, 0);
    inventory_add_item(ITEM_STICK, 32, 0);
}

void player_update(float delta_time) {
    if (g_player.stats.is_dead) {
        g_player.stats.death_time++;
        if (g_player.stats.death_time > 100) {
            player_respawn();
        }
        return;
    }
    
    if (g_player.stats.hurt_time > 0) {
        g_player.stats.hurt_time--;
    }
    
    if (g_player.stats.game_mode != GAME_MODE_CREATIVE && g_player.stats.game_mode != GAME_MODE_SPECTATOR) {
        if (g_player.fire_ticks > 0) {
            g_player.fire_ticks--;
            if (g_player.fire_ticks % 20 == 0) {
                player_take_damage(1.0f, DAMAGE_FIRE);
            }
        }
        
        g_player.stats.exhaustion += WALK_EXHAUSTION * delta_time * 60.0f;
        
        if (g_player.stats.exhaustion >= 4.0f) {
            g_player.stats.exhaustion = 0.0f;
            if (g_player.stats.saturation > 0.0f) {
                g_player.stats.saturation = fmaxf(0.0f, g_player.stats.saturation - 1.0f);
            } else if (g_player.stats.hunger > 0.0f) {
                g_player.stats.hunger = fmaxf(0.0f, g_player.stats.hunger - 1.0f);
            } else {
                player_take_damage(STARVATION_DAMAGE, DAMAGE_GENERIC);
            }
        }
        
        if (g_player.stats.hunger >= REGEN_HUNGER_THRESHOLD && g_player.stats.health < g_player.stats.max_health && g_player.stats.saturation > 0.0f) {
            g_player.stats.health = fminf(g_player.stats.max_health, g_player.stats.health + REGEN_RATE * delta_time);
            g_player.stats.saturation = fmaxf(0.0f, g_player.stats.saturation - REGEN_RATE * delta_time * 2.0f);
        }
        
        if (g_player.stats.hunger <= 0.0f) {
            player_take_damage(STARVATION_DAMAGE, DAMAGE_GENERIC);
        }
        
        if (g_player.eye_position.y < -10.0f) {
            player_take_damage(4.0f, DAMAGE_FALL);
        }
    }
    
    if (g_player.stats.game_mode == GAME_MODE_CREATIVE && g_player.stats.flying) {
        // Creative flight handled in input
    }
}

void player_take_damage(float amount, int damage_type) {
    (void)damage_type;
    if (g_player.stats.game_mode == GAME_MODE_CREATIVE || g_player.stats.game_mode == GAME_MODE_SPECTATOR) {
        return;
    }
    
    if (g_player.stats.hurt_time > 0) {
        return;
    }
    
    g_player.stats.health -= amount;
    g_player.stats.hurt_time = 10;
    
    if (g_player.stats.health <= 0.0f) {
        g_player.stats.health = 0.0f;
        g_player.stats.is_dead = true;
        g_player.stats.death_time = 0;
    }
}

void player_heal(float amount) {
    g_player.stats.health = fminf(g_player.stats.max_health, g_player.stats.health + amount);
}

void player_add_exhaustion(float amount) {
    if (g_player.stats.game_mode == GAME_MODE_CREATIVE || g_player.stats.game_mode == GAME_MODE_SPECTATOR) {
        return;
    }
    g_player.stats.exhaustion += amount;
}

void player_add_experience(int amount) {
    g_player.stats.experience_progress += amount;
    g_player.stats.experience_total += amount;
    
    int xp_for_next = g_player.stats.experience_level * 7 + (g_player.stats.experience_level > 15 ? (g_player.stats.experience_level - 15) * 3 : 0) + (g_player.stats.experience_level > 30 ? (g_player.stats.experience_level - 30) * 4 : 0) + 7;
    
    while (g_player.stats.experience_progress >= xp_for_next) {
        g_player.stats.experience_progress -= xp_for_next;
        g_player.stats.experience_level++;
        xp_for_next = g_player.stats.experience_level * 7 + (g_player.stats.experience_level > 15 ? (g_player.stats.experience_level - 15) * 3 : 0) + (g_player.stats.experience_level > 30 ? (g_player.stats.experience_level - 30) * 4 : 0) + 7;
    }
}

void player_set_game_mode(game_mode_t mode) {
    g_player.stats.game_mode = mode;
    
    if (mode == GAME_MODE_CREATIVE) {
        g_player.stats.flying = true;
        g_player.stats.health = BASE_HEALTH;
        g_player.stats.hunger = BASE_HUNGER;
        g_player.stats.saturation = BASE_SATURATION;
    } else if (mode == GAME_MODE_SPECTATOR) {
        g_player.stats.flying = true;
        g_player.stats.no_clip = true;
    } else {
        g_player.stats.flying = false;
        g_player.stats.no_clip = false;
    }
}

void player_toggle_creative(void) {
    if (g_player.stats.game_mode == GAME_MODE_CREATIVE) {
        player_set_game_mode(GAME_MODE_SURVIVAL);
    } else {
        player_set_game_mode(GAME_MODE_CREATIVE);
    }
}

void player_respawn(void) {
    g_player.stats.is_dead = false;
    g_player.stats.health = BASE_HEALTH;
    g_player.stats.hunger = BASE_HUNGER;
    g_player.stats.saturation = BASE_SATURATION;
    g_player.stats.exhaustion = 0.0f;
    g_player.stats.hurt_time = 0;
    g_player.stats.death_time = 0;
    g_player.stats.experience_level = 0;
    g_player.stats.experience_progress = 0;
    g_player.stats.experience_total = 0;
    
    if (g_player.stats.spawn_set) {
        main_camera_fov.position = g_player.stats.spawn_position;
    } else {
        main_camera_fov.position = (vector3){64.0f, 12.0f, 64.0f};
    }
    main_camera_fov.vertical_velocity = 0.0f;
    main_camera_fov.horizontal_velocity = vector3_zero();
}

void player_set_spawn(vector3 pos) {
    g_player.stats.spawn_position = pos;
    g_player.stats.spawn_set = true;
}

bool inventory_add_item(int item_id, int count, int damage) {
    if (item_id <= 0 || count <= 0) return false;
    
    const item_type* item = item_type_get(item_id);
    if (!item) return false;
    int max_stack = item->max_stack_size;
    
    for (int i = INV_SLOT_HOTBAR_START; i <= INV_SLOT_MAIN_END; i++) {
        item_stack_t* slot = &g_player.inventory.slots[i];
        if (slot->item_id == item_id && slot->damage == damage && slot->count < max_stack) {
            int can_add = max_stack - slot->count;
            int to_add = (count < can_add) ? count : can_add;
            slot->count += to_add;
            count -= to_add;
            if (count <= 0) return true;
        }
    }
    
    for (int i = INV_SLOT_HOTBAR_START; i <= INV_SLOT_MAIN_END; i++) {
        item_stack_t* slot = &g_player.inventory.slots[i];
        if (slot->item_id == ITEM_AIR || slot->count <= 0) {
            slot->item_id = item_id;
            slot->damage = damage;
            slot->count = (count < max_stack) ? count : max_stack;
            count -= slot->count;
            if (count <= 0) return true;
        }
    }
    
    return false;
}

bool inventory_remove_item(int item_id, int count) {
    if (item_id <= 0 || count <= 0) return false;
    
    for (int i = INV_SLOT_HOTBAR_START; i <= INV_SLOT_MAIN_END; i++) {
        item_stack_t* slot = &g_player.inventory.slots[i];
        if (slot->item_id == item_id && slot->count > 0) {
            int to_remove = (slot->count < count) ? slot->count : count;
            slot->count -= to_remove;
            count -= to_remove;
            if (slot->count <= 0) {
                slot->item_id = ITEM_AIR;
                slot->damage = 0;
            }
            if (count <= 0) return true;
        }
    }
    
    return false;
}

int inventory_get_count(int item_id) {
    int total = 0;
    for (int i = INV_SLOT_HOTBAR_START; i <= INV_SLOT_MAIN_END; i++) {
        if (g_player.inventory.slots[i].item_id == item_id) {
            total += g_player.inventory.slots[i].count;
        }
    }
    return total;
}

item_stack_t* inventory_get_slot(int slot_index) {
    if (slot_index < 0 || slot_index >= INVENTORY_TOTAL_SLOTS) return NULL;
    return &g_player.inventory.slots[slot_index];
}

void inventory_swap_slots(int from, int to) {
    if (from < 0 || from >= INVENTORY_TOTAL_SLOTS || to < 0 || to >= INVENTORY_TOTAL_SLOTS) return;
    if (from == to) return;
    
    item_stack_t temp = g_player.inventory.slots[from];
    g_player.inventory.slots[from] = g_player.inventory.slots[to];
    g_player.inventory.slots[to] = temp;
}

void inventory_move_to_hotbar(int from_slot) {
    if (from_slot < INV_SLOT_MAIN_START || from_slot > INV_SLOT_MAIN_END) return;
    
    item_stack_t* from = &g_player.inventory.slots[from_slot];
    if (from->item_id == ITEM_AIR) return;
    
    for (int i = INV_SLOT_HOTBAR_START; i <= INV_SLOT_HOTBAR_END; i++) {
        item_stack_t* to = &g_player.inventory.slots[i];
        if (to->item_id == ITEM_AIR) {
            *to = *from;
            from->item_id = ITEM_AIR;
            from->count = 0;
            from->damage = 0;
            return;
        } else if (to->item_id == from->item_id && to->damage == from->damage) {
            const item_type* item = item_type_get(to->item_id);
            int max_stack = item ? item->max_stack_size : MAX_STACK_SIZE;
            int space = max_stack - to->count;
            if (space > 0) {
                int move = (from->count < space) ? from->count : space;
                to->count += move;
                from->count -= move;
                if (from->count <= 0) {
                    from->item_id = ITEM_AIR;
                    from->damage = 0;
                }
                if (from->count <= 0) return;
            }
        }
    }
}

void inventory_select_hotbar_slot(int slot) {
    if (slot >= 0 && slot <= 8) {
        g_player.inventory.selected_hotbar_slot = slot;
    }
}

void inventory_open_close(void) {
    g_player.inventory.inventory_open = !g_player.inventory.inventory_open;
    if (!g_player.inventory.inventory_open) {
        g_player.inventory.crafting_table_open = false;
    }
}

void inventory_crafting_table_open_close(void) {
    g_player.inventory.crafting_table_open = !g_player.inventory.crafting_table_open;
    if (g_player.inventory.crafting_table_open) {
        g_player.inventory.inventory_open = true;
    }
}

void inventory_crafting_grid_craft(void) {
    // TODO: Implement crafting logic
}

void inventory_clear_crafting_grid(void) {
    for (int i = INV_SLOT_CRAFTING_GRID_START; i <= INV_SLOT_CRAFTING_GRID_END; i++) {
        g_player.inventory.slots[i].item_id = ITEM_AIR;
        g_player.inventory.slots[i].count = 0;
        g_player.inventory.slots[i].damage = 0;
    }
    g_player.inventory.slots[INV_SLOT_CRAFTING_RESULT].item_id = ITEM_AIR;
    g_player.inventory.slots[INV_SLOT_CRAFTING_RESULT].count = 0;
    g_player.inventory.slots[INV_SLOT_CRAFTING_RESULT].damage = 0;
}

bool player_can_break_block(int block_id, tool_type_t tool, int tool_tier) {
    (void)tool_tier;
    const block_type* bt = block_type_get(block_id);
    if (!bt) return false;
    if (bt->hardness < 0) return false;
    if (bt->hardness == 0) return true;
    return block_is_tool_effective(block_id, tool);
}

float player_get_break_speed(int block_id, tool_type_t tool, int tool_tier) {
    (void)tool_tier;
    const block_type* bt = block_type_get(block_id);
    if (!bt) return 0.0f;
    if (bt->hardness == 0) return 100.0f;
    if (bt->hardness < 0) return 0.0f;
    
    float speed = 1.0f / bt->hardness;
    
    if (tool != TOOL_NONE && block_is_tool_effective(block_id, tool)) {
        float tool_multiplier = 1.0f;
        switch (tool) {
            case TOOL_PICKAXE:
                if (tool_tier >= 0 && tool_tier < 6) tool_multiplier = bt->break_speed_pickaxe[tool_tier];
                break;
            case TOOL_AXE:
                if (tool_tier >= 0 && tool_tier < 6) tool_multiplier = bt->break_speed_axe[tool_tier];
                break;
            case TOOL_SHOVEL:
                if (tool_tier >= 0 && tool_tier < 6) tool_multiplier = bt->break_speed_shovel[tool_tier];
                break;
            case TOOL_HOE:
                if (tool_tier >= 0 && tool_tier < 6) tool_multiplier = bt->break_speed_hoe[tool_tier];
                break;
            case TOOL_SHEARS:
                if (tool_tier >= 0 && tool_tier < 6) tool_multiplier = bt->break_speed_shears[tool_tier];
                break;
            default: break;
        }
        speed *= tool_multiplier;
    }
    
    if (!g_player.on_ground) speed /= 5.0f;
    if (g_player.stats.flying && g_player.stats.game_mode != GAME_MODE_CREATIVE) speed /= 5.0f;
    
    return speed;
}

int player_get_item_in_hand(void) {
    int slot = INV_SLOT_HOTBAR_START + g_player.inventory.selected_hotbar_slot;
    return g_player.inventory.slots[slot].item_id;
}

void player_swing_hand(void) {
    // TODO: Hand swing animation
}