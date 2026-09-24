#include "item_registry.h"
#include "block.h"
#include <string.h>

static item_type item_registry[ITEM_TYPE_MAX];
static int item_registry_count = 0;

void item_registry_init(void) {
    item_registry_count = 0;
    memset(item_registry, 0, sizeof(item_registry));
    
    item_type_register("air", -1, 0.0f, 0.0f, 0.0f, 1, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    
    item_type_register("oak_log", BLOCK_OAK_LOG, 0.60f, 0.40f, 0.18f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    item_type_register("dirt", BLOCK_DIRT, 0.60f, 0.42f, 0.25f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    item_type_register("stone", BLOCK_STONE, 0.55f, 0.55f, 0.55f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    item_type_register("oak_planks", BLOCK_OAK_PLANKS, 0.75f, 0.58f, 0.32f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    item_type_register("sand", BLOCK_SAND, 0.90f, 0.85f, 0.60f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    item_type_register("cobblestone", BLOCK_COBBLESTONE, 0.50f, 0.50f, 0.50f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    item_type_register("oak_sapling", BLOCK_OAK_SAPLING, 0.25f, 0.65f, 0.18f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    
    item_type_register("stick", -1, 0.60f, 0.40f, 0.18f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    
    item_type_register("wooden_pickaxe", -1, 0.60f, 0.40f, 0.18f, 1, 59, TOOL_PICKAXE, 0, 2.0f, 1.2f, 0, 0.0f, false, false);
    item_type_register("wooden_axe", -1, 0.60f, 0.40f, 0.18f, 1, 59, TOOL_AXE, 0, 3.0f, 0.8f, 0, 0.0f, false, false);
    item_type_register("wooden_shovel", -1, 0.60f, 0.40f, 0.18f, 1, 59, TOOL_SHOVEL, 0, 1.5f, 1.0f, 0, 0.0f, false, false);
    item_type_register("wooden_hoe", -1, 0.60f, 0.40f, 0.18f, 1, 59, TOOL_HOE, 0, 1.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("wooden_sword", -1, 0.60f, 0.40f, 0.18f, 1, 59, TOOL_SWORD, 0, 4.0f, 1.6f, 0, 0.0f, false, false);
    
    item_type_register("stone_pickaxe", -1, 0.55f, 0.55f, 0.55f, 1, 131, TOOL_PICKAXE, 1, 3.0f, 1.2f, 0, 0.0f, false, false);
    item_type_register("stone_axe", -1, 0.55f, 0.55f, 0.55f, 1, 131, TOOL_AXE, 1, 4.0f, 0.8f, 0, 0.0f, false, false);
    item_type_register("stone_shovel", -1, 0.55f, 0.55f, 0.55f, 1, 131, TOOL_SHOVEL, 1, 2.5f, 1.0f, 0, 0.0f, false, false);
    item_type_register("stone_hoe", -1, 0.55f, 0.55f, 0.55f, 1, 131, TOOL_HOE, 1, 2.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("stone_sword", -1, 0.55f, 0.55f, 0.55f, 1, 131, TOOL_SWORD, 1, 5.0f, 1.6f, 0, 0.0f, false, false);
    
    item_type_register("iron_ingot", -1, 0.75f, 0.75f, 0.78f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("gold_ingot", -1, 0.95f, 0.85f, 0.25f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("diamond", -1, 0.35f, 0.90f, 0.95f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("coal", -1, 0.30f, 0.30f, 0.30f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    
    item_type_register("iron_pickaxe", -1, 0.75f, 0.75f, 0.78f, 1, 250, TOOL_PICKAXE, 2, 4.0f, 1.2f, 0, 0.0f, false, false);
    item_type_register("iron_axe", -1, 0.75f, 0.75f, 0.78f, 1, 250, TOOL_AXE, 2, 5.0f, 0.9f, 0, 0.0f, false, false);
    item_type_register("iron_shovel", -1, 0.75f, 0.75f, 0.78f, 1, 250, TOOL_SHOVEL, 2, 3.5f, 1.0f, 0, 0.0f, false, false);
    item_type_register("iron_hoe", -1, 0.75f, 0.75f, 0.78f, 1, 250, TOOL_HOE, 2, 3.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("iron_sword", -1, 0.75f, 0.75f, 0.78f, 1, 250, TOOL_SWORD, 2, 6.0f, 1.6f, 0, 0.0f, false, false);
    
    item_type_register("diamond_pickaxe", -1, 0.35f, 0.90f, 0.95f, 1, 1561, TOOL_PICKAXE, 3, 5.0f, 1.2f, 0, 0.0f, false, false);
    item_type_register("diamond_axe", -1, 0.35f, 0.90f, 0.95f, 1, 1561, TOOL_AXE, 3, 6.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("diamond_shovel", -1, 0.35f, 0.90f, 0.95f, 1, 1561, TOOL_SHOVEL, 3, 4.5f, 1.0f, 0, 0.0f, false, false);
    item_type_register("diamond_hoe", -1, 0.35f, 0.90f, 0.95f, 1, 1561, TOOL_HOE, 3, 4.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("diamond_sword", -1, 0.35f, 0.90f, 0.95f, 1, 1561, TOOL_SWORD, 3, 7.0f, 1.6f, 0, 0.0f, false, false);
    
    item_type_register("bread", -1, 0.90f, 0.75f, 0.40f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 5, 6.0f, true, false);
    item_type_register("apple", -1, 0.80f, 0.20f, 0.20f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 4, 2.4f, true, false);
    item_type_register("cooked_beef", -1, 0.70f, 0.40f, 0.20f, 64, 0, TOOL_NONE, 0, 0.0f, 1.0f, 8, 12.8f, true, false);
    
    item_type_register("bucket", -1, 0.70f, 0.70f, 0.70f, 16, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("water_bucket", -1, 0.25f, 0.48f, 0.95f, 1, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    item_type_register("lava_bucket", -1, 0.95f, 0.45f, 0.15f, 1, 0, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, true);
    
    item_type_register("flint_and_steel", -1, 0.50f, 0.50f, 0.50f, 1, 64, TOOL_NONE, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
    item_type_register("shears", -1, 0.70f, 0.70f, 0.70f, 1, 238, TOOL_SHEARS, 0, 0.0f, 1.0f, 0, 0.0f, false, false);
}

int item_type_register(const char* name, int block_id, float r, float g, float b,
                          int max_stack, int max_damage, tool_type_t tool, int tier,
                          float atk_dmg, float atk_spd, int food, float saturation,
                          bool is_food, bool placeable) {
    if (item_registry_count >= ITEM_TYPE_MAX) return -1;
    int id = item_registry_count;
    item_type *it = &item_registry[id];
    it->name = name;
    it->block_id = block_id;
    it->colour_r = r;
    it->colour_g = g;
    it->colour_b = b;
    it->max_stack_size = max_stack;
    it->max_damage = max_damage;
    it->tool_type = tool;
    it->tool_tier = tier;
    it->attack_damage = atk_dmg;
    it->attack_speed = atk_spd;
    it->food_value = food;
    it->saturation = saturation;
    it->is_food = is_food;
    it->placeable = placeable;
    item_registry_count++;
    return id;
}

const item_type* item_type_get(int item_id) {
    if (item_id < 0 || item_id >= item_registry_count) return &item_registry[ITEM_AIR];
    return &item_registry[item_id];
}

int item_type_count(void) {
    return item_registry_count;
}