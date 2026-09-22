/* PotatoWorld v2 — Block Registry Implementation */

#include "block.h"
#include <string.h>

static block_type registry [BLOCK_TYPE_MAX];
static int registry_count = 0;

void block_registry_init (void) {
    registry_count = 0;
    memset (registry, 0, sizeof (registry));

    /* BLOCK_AIR must be first */
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
}

int block_type_register (const char *name, float r, float g, float b,
                         float fric_static, float fric_kinetic,
                         float restitution, bool solid, bool transparent) {
    if (registry_count >= BLOCK_TYPE_MAX) {return -1;}
    int id = registry_count;
    block_type *bt = &registry [id];
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
