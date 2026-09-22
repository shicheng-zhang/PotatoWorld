/* PotatoWorld v2 — Block Registry
 * 15 block types with per-type physics properties. */

#ifndef game_block_h
#define game_block_h

#include <stdbool.h>

#define BLOCK_TYPE_MAX 64

typedef struct {
    const char *name;
    float colour_r, colour_g, colour_b;
    float friction_static;
    float friction_kinetic;
    float restitution;
    bool solid;
    bool transparent;
} block_type;

void block_registry_init (void);
int block_type_register (const char *name, float r, float g, float b,
                         float fric_static, float fric_kinetic,
                         float restitution, bool solid, bool transparent);
const block_type *block_type_get (int block_id);
int block_type_count (void);

/* Built-in block type IDs */
#define BLOCK_AIR       0
#define BLOCK_GRASS     1
#define BLOCK_DIRT      2
#define BLOCK_STONE     3
#define BLOCK_WOOD      4
#define BLOCK_SAND      5
#define BLOCK_WATER     6
#define BLOCK_PLANK     7
#define BLOCK_COBBLE    8
#define BLOCK_BRICK     9
#define BLOCK_GLASS    10
#define BLOCK_LEAVES   11
#define BLOCK_IRON     12
#define BLOCK_GOLD     13
#define BLOCK_DIAMOND  14

#endif
