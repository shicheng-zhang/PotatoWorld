/* v10S — Block Registry with Behaviors
 * Extended for Minecraft-like gameplay. */

#ifndef game_block_h
#define game_block_h

#include <stdbool.h>
#include <stdint.h>
#include "../core/math3D.h"

#define BLOCK_TYPE_MAX 256
#define MAX_BLOCK_TAGS 32

/* Block tags for tool effectiveness and behavior grouping */
typedef enum {
    TAG_NONE = 0,
    TAG_DIRT     = 1 << 0,
    TAG_GRASS    = 1 << 1,
    TAG_STONE    = 1 << 2,
    TAG_WOOD     = 1 << 3,
    TAG_SAND     = 1 << 4,
    TAG_WATER    = 1 << 5,
    TAG_LEAVES   = 1 << 6,
    TAG_WOOL     = 1 << 7,
    TAG_GLASS    = 1 << 8,
    TAG_ORE      = 1 << 9,
    TAG_METAL    = 1 << 10,
    TAG_GEM      = 1 << 11,
    TAG_PLANT    = 1 << 12,
    TAG_FLAMMABLE = 1 << 13,
    TAG_FALLING  = 1 << 14,
    TAG_FLUID    = 1 << 15,
    TAG_TRANSPARENT = 1 << 16,
    TAG_CROP     = 1 << 17,
    TAG_FARMLAND = 1 << 18,
} block_tag_t;

/* Tool types for break speed calculation */
typedef enum {
    TOOL_NONE = 0,
    TOOL_PICKAXE,
    TOOL_AXE,
    TOOL_SHOVEL,
    TOOL_HOE,
    TOOL_SWORD,
    TOOL_SHEARS,
} tool_type_t;

/* Forward declarations */
typedef struct game_world game_world;
typedef struct game_world game_world;

/* Block behavior callbacks */
typedef void (*block_on_place_fn)(game_world *w, int x, int y, int z);
typedef void (*block_on_break_fn)(game_world *w, int x, int y, int z, tool_type_t tool);
typedef void (*block_on_tick_fn)(game_world *w, int x, int y, int z);
typedef void (*block_on_neighbor_change_fn)(game_world *w, int x, int y, int z, int nx, int ny, int nz);
typedef void (*block_on_random_tick_fn)(game_world *w, int x, int y, int z);
typedef bool (*block_can_survive_fn)(game_world *w, int x, int y, int z);
typedef int  (*block_get_drop_fn)(int block_id, tool_type_t tool, int fortune_level, bool silk_touch);
typedef int  (*block_get_break_speed_fn)(int block_id, tool_type_t tool, int tool_tier);

/* Block type definition with full behavior support */
typedef struct {
    const char *name;
    float colour_r, colour_g, colour_b;
    float friction_static;
    float friction_kinetic;
    float restitution;
    bool solid;
    bool transparent;
    
    /* Tags for grouping */
    uint32_t tags;
    
    /* Light properties */
    int light_emission;      /* 0-15 */
    int light_opacity;       /* 0-15, 0 = transparent to light */
    
    /* Hardness & mining */
    float hardness;          /* Base break time in seconds at efficiency 1 */
    float blast_resistance;  /* Explosion resistance */
    
    /* Tool effectiveness */
    float break_speed_pickaxe[6];  /* Tier 0-5 (wood to netherite) */
    float break_speed_axe[6];
    float break_speed_shovel[6];
    float break_speed_hoe[6];
    float break_speed_sword[6];
    float break_speed_shears[6];
    
    /* Behavior callbacks (NULL = default/no-op) */
    block_on_place_fn on_place;
    block_on_break_fn on_break;
    block_on_tick_fn on_tick;
    block_on_neighbor_change_fn on_neighbor_change;
    block_on_random_tick_fn on_random_tick;
    block_can_survive_fn can_survive;
    block_get_drop_fn get_drop;
    block_get_break_speed_fn get_break_speed;
    
    /* Fluid properties */
    int fluid_level;         /* 0 = not fluid, 1-8 = flowing, 8 = source */
    bool is_source;          /* True for water/lava source blocks */
    
    /* Crop properties */
    int max_age;             /* 0 = not a crop */
    int growth_ticks;        /* Ticks per growth stage (randomized) */
    
} block_type;

/* Block registry */
void block_registry_init(void);
int block_type_register(const char* name, float r, float g, float b,
                             float fric_static, float fric_kinetic, float restitution,
                             bool solid, bool transparent);
const block_type *block_type_get(int block_id);
int block_type_count(void);

/* Tag helpers */
static inline bool block_has_tag(int block_id, uint32_t tag) {
    const block_type *bt = block_type_get(block_id);
    return bt && (bt->tags & tag);
}
static inline bool block_is_tool_effective(int block_id, tool_type_t tool) {
    const block_type *bt = block_type_get(block_id);
    if (!bt) return false;
    switch (tool) {
        case TOOL_PICKAXE: return (bt->tags & (TAG_STONE | TAG_ORE | TAG_METAL | TAG_GEM)) != 0;
        case TOOL_AXE:     return (bt->tags & (TAG_WOOD | TAG_PLANT)) != 0;
        case TOOL_SHOVEL:  return (bt->tags & (TAG_DIRT | TAG_GRASS | TAG_SAND)) != 0;
        case TOOL_HOE:     return (bt->tags & (TAG_FARMLAND | TAG_DIRT | TAG_GRASS)) != 0;
        case TOOL_SHEARS:  return (bt->tags & (TAG_LEAVES | TAG_WOOL)) != 0;
        default: return false;
    }
}

/* Built-in block type IDs (extended) */
#define BLOCK_AIR            0
#define BLOCK_GRASS_BLOCK    1
#define BLOCK_GRASS           BLOCK_GRASS_BLOCK
#define BLOCK_DIRT           2
#define BLOCK_STONE          3
#define BLOCK_OAK_LOG        4
#define BLOCK_SAND           5
#define BLOCK_WATER          6
#define BLOCK_OAK_PLANKS     7
#define BLOCK_COBBLESTONE    8
#define BLOCK_BRICKS         9
#define BLOCK_GLASS          10
#define BLOCK_OAK_LEAVES     11
#define BLOCK_IRON_ORE       12
#define BLOCK_GOLD_ORE       13
#define BLOCK_DIAMOND_ORE    14
#define BLOCK_OAK_SAPLING    15
#define BLOCK_CRAFTING_TABLE 16
#define BLOCK_FURNACE        17
#define BLOCK_CHEST          18
#define BLOCK_FARMLAND       19
#define BLOCK_WHEAT          20
#define BLOCK_TORCH          21
#define BLOCK_COAL_ORE       22
#define BLOCK_IRON_BLOCK     23
#define BLOCK_GOLD_BLOCK     24
#define BLOCK_DIAMOND_BLOCK  25
#define BLOCK_BEDROCK        26
#define BLOCK_GRAVEL         27
#define BLOCK_LAVA           28
#define BLOCK_OBSIDIAN       29
#define BLOCK_TNT            30

/* Built-in item IDs */
#define ITEM_AIR             0
#define ITEM_OAK_LOG         1
#define ITEM_DIRT            2
#define ITEM_STONE           3
#define ITEM_OAK_PLANKS      4
#define ITEM_SAND            5
#define ITEM_COBBLESTONE     6
#define ITEM_OAK_SAPLING     7
#define ITEM_STICK           8
#define ITEM_WOODEN_PICKAXE  9
#define ITEM_WOODEN_AXE      10
#define ITEM_WOODEN_SHOVEL   11
#define ITEM_WOODEN_HOE      12
#define ITEM_WOODEN_SWORD    13
#define ITEM_STONE_PICKAXE   14
#define ITEM_STONE_AXE       15
#define ITEM_STONE_SHOVEL    16
#define ITEM_STONE_HOE       17
#define ITEM_STONE_SWORD     18
#define ITEM_IRON_INGOT      19
#define ITEM_GOLD_INGOT      20
#define ITEM_DIAMOND         21
#define ITEM_COAL            22
#define ITEM_IRON_PICKAXE    23
#define ITEM_IRON_AXE        24
#define ITEM_IRON_SHOVEL     25
#define ITEM_IRON_HOE        26
#define ITEM_IRON_SWORD      27
#define ITEM_DIAMOND_PICKAXE 28
#define ITEM_DIAMOND_AXE     29
#define ITEM_DIAMOND_SHOVEL  30
#define ITEM_DIAMOND_HOE     31
#define ITEM_DIAMOND_SWORD   32
#define ITEM_BREAD           33
#define ITEM_APPLE           34
#define ITEM_COOKED_BEEF     35
#define ITEM_BUCKET          36
#define ITEM_WATER_BUCKET    37
#define ITEM_LAVA_BUCKET     38
#define ITEM_FLINT_AND_STEEL 39
#define ITEM_SHEARS          40

#endif