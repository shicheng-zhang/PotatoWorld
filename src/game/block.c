/* v10S — Block Registry Implementation
 * Minecraft-like hardness, tags, and physics properties. */

#include "block.h"
#include <string.h>

static block_type registry [BLOCK_TYPE_MAX];
static int registry_count = 0;

void block_registry_init (void) {
    registry_count = 0;
    memset (registry, 0, sizeof (registry));

    /* BLOCK_AIR must be first — hardness 0, not solid */
    block_type_register ("air",       0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, false, true);
    block_type_register ("grass",     0.33f, 0.75f, 0.23f,  0.6f, 0.5f, 0.1f, true, false);
    block_type_register ("dirt",      0.60f, 0.42f, 0.25f, 0.5f, 0.4f, 0.1f, true, false);
    block_type_register ("stone",     0.55f, 0.55f, 0.55f,  0.8f, 0.7f, 0.0f, true, false);
    block_type_register ("wood",      0.60f, 0.40f, 0.18f, 0.6f, 0.5f, 0.1f, true, false);
    block_type_register ("sand",      0.90f, 0.85f, 0.60f, 0.4f, 0.3f, 0.0f, true, false);
    block_type_register ("water",     0.25f, 0.48f, 0.95f, 0.1f, 0.1f, 0.3f, false, true);
    block_type_register ("plank",     0.75f, 0.58f, 0.32f, 0.6f, 0.5f, 0.1f, true, false);
    block_type_register ("cobble",    0.50f, 0.50f, 0.50f, 0.8f, 0.7f, 0.0f, true, false);
    block_type_register ("brick",     0.75f, 0.32f, 0.22f, 0.8f, 0.7f, 0.0f, true, false);
    block_type_register ("glass",     0.82f, 0.92f, 1.0f,  0.2f, 0.1f, 0.0f, true, true);
    block_type_register ("leaves",    0.25f, 0.65f, 0.18f, 0.3f, 0.2f, 0.2f, true, true);
    block_type_register ("iron",      0.75f, 0.75f, 0.78f, 0.9f, 0.8f, 0.0f, true, false);
    block_type_register ("gold",      0.95f, 0.85f, 0.25f,  0.8f, 0.7f, 0.0f, true, false);
    block_type_register ("diamond",   0.35f, 0.90f, 0.95f, 0.95f, 0.9f, 0.0f, true, false);
    /* extended minecrafty palette */
    block_type_register ("sapling",   0.25f, 0.65f, 0.18f, 0.0f, 0.0f, 0.0f, false, true); // 15
    block_type_register ("crafting",  0.70f, 0.55f, 0.35f, 0.6f, 0.5f, 0.0f, true, false); // 16
    block_type_register ("furnace",   0.45f, 0.45f, 0.45f, 0.8f, 0.7f, 0.0f, true, false); // 17
    block_type_register ("chest",     0.75f, 0.55f, 0.25f, 0.6f, 0.5f, 0.0f, true, false); // 18
    block_type_register ("farmland",  0.55f, 0.38f, 0.22f, 0.5f, 0.4f, 0.0f, true, false); // 19
    block_type_register ("wheat",     0.85f, 0.80f, 0.20f, 0.0f, 0.0f, 0.0f, false, true); // 20
    block_type_register ("torch",     0.95f, 0.85f, 0.30f, 0.0f, 0.0f, 0.0f, false, true); // 21
    block_type_register ("coal_ore",  0.20f, 0.20f, 0.20f, 0.8f, 0.7f, 0.0f, true, false); // 22
    block_type_register ("iron_block",0.78f, 0.78f, 0.80f, 0.9f, 0.8f, 0.0f, true, false); // 23
    block_type_register ("gold_block",0.95f, 0.85f, 0.25f, 0.9f, 0.8f, 0.0f, true, false); // 24
    block_type_register ("diamond_block",0.35f,0.90f,0.95f,0.95f,0.9f,0.0f,true,false); //25
    block_type_register ("bedrock",   0.15f, 0.15f, 0.15f, 0.8f, 0.7f, 0.0f, true, false); // 26
    block_type_register ("gravel",    0.55f, 0.52f, 0.50f, 0.5f, 0.4f, 0.0f, true, false); // 27
    block_type_register ("lava",      0.90f, 0.30f, 0.05f, 0.1f, 0.1f, 0.0f, false,true); //28
    block_type_register ("obsidian",  0.10f, 0.08f, 0.22f, 0.9f, 0.8f, 0.0f, true, false); //29
    block_type_register ("tnt",       0.85f, 0.15f, 0.10f, 0.5f, 0.4f, 0.0f, true, false); //30

    /* Minecraft-like hardness & tags — critical for hold-to-break timing */
    registry[BLOCK_AIR].hardness = 0.0f; registry[BLOCK_AIR].blast_resistance = 0.0f; registry[BLOCK_AIR].tags = TAG_NONE; registry[BLOCK_AIR].transparent = true;
    registry[BLOCK_GRASS].hardness = 0.6f; registry[BLOCK_GRASS].blast_resistance = 0.6f; registry[BLOCK_GRASS].tags = TAG_GRASS | TAG_DIRT;
    registry[BLOCK_DIRT].hardness = 0.5f; registry[BLOCK_DIRT].blast_resistance = 0.5f; registry[BLOCK_DIRT].tags = TAG_DIRT;
    registry[BLOCK_STONE].hardness = 1.5f; registry[BLOCK_STONE].blast_resistance = 6.0f; registry[BLOCK_STONE].tags = TAG_STONE;
    registry[BLOCK_OAK_LOG].hardness = 2.0f; registry[BLOCK_OAK_LOG].blast_resistance = 2.0f; registry[BLOCK_OAK_LOG].tags = TAG_WOOD | TAG_FLAMMABLE;
    registry[BLOCK_SAND].hardness = 0.5f; registry[BLOCK_SAND].blast_resistance = 0.5f; registry[BLOCK_SAND].tags = TAG_SAND | TAG_FALLING;
    registry[BLOCK_WATER].hardness = 100.0f; registry[BLOCK_WATER].blast_resistance = 100.0f; registry[BLOCK_WATER].tags = TAG_WATER | TAG_FLUID | TAG_TRANSPARENT | TAG_FALLING; registry[BLOCK_WATER].transparent = true; registry[BLOCK_WATER].fluid_level=8; registry[BLOCK_WATER].is_source=true;
    registry[BLOCK_OAK_PLANKS].hardness = 2.0f; registry[BLOCK_OAK_PLANKS].blast_resistance = 3.0f; registry[BLOCK_OAK_PLANKS].tags = TAG_WOOD | TAG_FLAMMABLE;
    registry[BLOCK_COBBLESTONE].hardness = 2.0f; registry[BLOCK_COBBLESTONE].blast_resistance = 6.0f; registry[BLOCK_COBBLESTONE].tags = TAG_STONE;
    registry[BLOCK_BRICKS].hardness = 2.0f; registry[BLOCK_BRICKS].blast_resistance = 6.0f; registry[BLOCK_BRICKS].tags = TAG_STONE;
    registry[BLOCK_GLASS].hardness = 0.3f; registry[BLOCK_GLASS].blast_resistance = 0.3f; registry[BLOCK_GLASS].tags = TAG_GLASS | TAG_TRANSPARENT;
    registry[BLOCK_OAK_LEAVES].hardness = 0.2f; registry[BLOCK_OAK_LEAVES].blast_resistance = 0.2f; registry[BLOCK_OAK_LEAVES].tags = TAG_LEAVES | TAG_TRANSPARENT | TAG_FLAMMABLE; registry[BLOCK_OAK_LEAVES].transparent = true;
    registry[BLOCK_IRON_ORE].hardness = 3.0f; registry[BLOCK_IRON_ORE].blast_resistance = 3.0f; registry[BLOCK_IRON_ORE].tags = TAG_ORE | TAG_STONE;
    registry[BLOCK_GOLD_ORE].hardness = 3.0f; registry[BLOCK_GOLD_ORE].blast_resistance = 3.0f; registry[BLOCK_GOLD_ORE].tags = TAG_ORE | TAG_METAL;
    registry[BLOCK_DIAMOND_ORE].hardness = 3.0f; registry[BLOCK_DIAMOND_ORE].blast_resistance = 3.0f; registry[BLOCK_DIAMOND_ORE].tags = TAG_ORE | TAG_GEM;
    registry[BLOCK_OAK_SAPLING].hardness = 0.0f; registry[BLOCK_OAK_SAPLING].blast_resistance = 0.0f; registry[BLOCK_OAK_SAPLING].tags = TAG_PLANT | TAG_FLAMMABLE; registry[BLOCK_OAK_SAPLING].transparent=true;
    registry[BLOCK_CRAFTING_TABLE].hardness = 2.5f; registry[BLOCK_CRAFTING_TABLE].blast_resistance=2.5f; registry[BLOCK_CRAFTING_TABLE].tags=TAG_WOOD;
    registry[BLOCK_FURNACE].hardness=3.5f; registry[BLOCK_FURNACE].blast_resistance=3.5f; registry[BLOCK_FURNACE].tags=TAG_STONE;
    registry[BLOCK_CHEST].hardness=2.5f; registry[BLOCK_CHEST].blast_resistance=2.5f; registry[BLOCK_CHEST].tags=TAG_WOOD;
    registry[BLOCK_FARMLAND].hardness=0.6f; registry[BLOCK_FARMLAND].blast_resistance=0.6f; registry[BLOCK_FARMLAND].tags=TAG_DIRT|TAG_FARMLAND;
    registry[BLOCK_WHEAT].hardness=0.0f; registry[BLOCK_WHEAT].tags=TAG_CROP|TAG_PLANT; registry[BLOCK_WHEAT].transparent=true;
    registry[BLOCK_TORCH].hardness=0.0f; registry[BLOCK_TORCH].tags=TAG_NONE; registry[BLOCK_TORCH].transparent=true; registry[BLOCK_TORCH].light_emission=14;
    registry[BLOCK_COAL_ORE].hardness=3.0f; registry[BLOCK_COAL_ORE].blast_resistance=3.0f; registry[BLOCK_COAL_ORE].tags=TAG_ORE|TAG_STONE;
    registry[BLOCK_IRON_BLOCK].hardness=5.0f; registry[BLOCK_IRON_BLOCK].tags=TAG_METAL;
    registry[BLOCK_GOLD_BLOCK].hardness=3.0f; registry[BLOCK_GOLD_BLOCK].tags=TAG_METAL;
    registry[BLOCK_DIAMOND_BLOCK].hardness=5.0f; registry[BLOCK_DIAMOND_BLOCK].tags=TAG_GEM;
    registry[BLOCK_BEDROCK].hardness=-1.0f; registry[BLOCK_BEDROCK].blast_resistance=18000000.0f; registry[BLOCK_BEDROCK].tags=TAG_STONE;
    registry[BLOCK_GRAVEL].hardness=0.6f; registry[BLOCK_GRAVEL].tags=TAG_SAND|TAG_FALLING; registry[BLOCK_GRAVEL].transparent=false;
    registry[BLOCK_LAVA].hardness=100.0f; registry[BLOCK_LAVA].tags=TAG_FLUID|TAG_FALLING; registry[BLOCK_LAVA].transparent=true; registry[BLOCK_LAVA].fluid_level=8; registry[BLOCK_LAVA].is_source=true; registry[BLOCK_LAVA].light_emission=15;
    registry[BLOCK_OBSIDIAN].hardness=50.0f; registry[BLOCK_OBSIDIAN].blast_resistance=1200.0f; registry[BLOCK_OBSIDIAN].tags=TAG_STONE;
    registry[BLOCK_TNT].hardness=0.0f; registry[BLOCK_TNT].tags=TAG_NONE;
}

int block_type_register (const char *name, float r, float g, float b,
                          float fric_static, float fric_kinetic,
                          float restitution, bool solid, bool transparent) {
    if (registry_count >= BLOCK_TYPE_MAX) {return -1;}
    int id = registry_count;
    block_type *bt = &registry [id];
    memset(bt, 0, sizeof(block_type));
    bt->name = name;
    bt->colour_r = r;
    bt->colour_g = g;
    bt->colour_b = b;
    bt->friction_static = fric_static;
    bt->friction_kinetic = fric_kinetic;
    bt->restitution = restitution;
    bt->solid = solid;
    bt->transparent = transparent;
    registry_count++;
    return id;
}

const block_type *block_type_get (int block_id) {
    if (block_id < 0 || block_id >= registry_count) {return &registry [BLOCK_AIR];}
    return &registry [block_id];
}

int block_type_count (void) {
    return registry_count;
}
