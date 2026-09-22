/* PotatoWorld v2 — Game Init
 * World generation, block interaction, player/world collision.
 * Additive layer on MPE v14S. No engine physics code modified. */

#ifndef game_init_h
#define game_init_h

#include "world.h"
#include "raycast.h"

extern game_world pw_world;
extern int pw_selected_block;
extern int pw_block_type_count;

void game_init (void);
void game_update (float delta_time, float cam_x, float cam_y, float cam_z,
                  float dir_x, float dir_y, float dir_z);
void game_on_left_click (float cam_x, float cam_y, float cam_z,
                         float dir_x, float dir_y, float dir_z);
void game_on_right_click (float cam_x, float cam_y, float cam_z,
                          float dir_x, float dir_y, float dir_z);
void game_scroll_block (int direction);

/* Player-world collision: 3-sphere player against voxel grid.
 * Modifies cam_x/y/z in place. Returns true if grounded. */
bool game_player_collide (float *cam_x, float *cam_y, float *cam_z,
                          float *vert_vel, float *horiz_vx, float *horiz_vz);

/* Rigidbody-voxel collision: push dynamic rigidbodies out of solid blocks. */
void game_rigidbody_voxel_collide (void);

#endif
