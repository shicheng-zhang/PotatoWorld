/* PotatoWorld v2 — DDA Voxel Raycast
 * Block-type-aware ray intersection, returns hit position + face normal. */

#ifndef game_raycast_h
#define game_raycast_h

#include "world.h"
#include <stdbool.h>

typedef struct {
    int block_x, block_y, block_z;
    int normal_x, normal_y, normal_z;
    float distance;
    bool hit;
} raycast_result;

raycast_result raycast_voxel (const game_world *w,
                              float origin_x, float origin_y, float origin_z,
                              float dir_x, float dir_y, float dir_z,
                              float max_distance);

#endif
