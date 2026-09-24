/* v10S — Game Init
 * World generation, block interaction, player/world collision.
 * Additive layer on v10S. No engine physics code modified. */

#ifndef game_init_h
#define game_init_h

#include "world.h"
#include "raycast.h"

/* Minecraft player dimensions: 0.6 wide, 1.8 tall, eye 1.62 */
#define PLAYER_WIDTH 0.6f
#define PLAYER_HEIGHT 1.8f
#define PLAYER_EYE_HEIGHT 1.62f
#define PLAYER_HALF_W (PLAYER_WIDTH * 0.5f)
#define PLAYER_HALF_H (PLAYER_HEIGHT * 0.5f)
#define STEP_HEIGHT 0.6f
#define BREAK_MAX_DISTANCE 8.0f

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

/* Hold-to-break tick: call every physics frame while left held */
void game_tick_break (float dt, float cam_x, float cam_y, float cam_z,
                      float dir_x, float dir_y, float dir_z, bool is_holding);
void game_tick_falling (void);

/* Player-world collision: AABB (0.6x1.8) player against voxel grid.
 * Modifies cam_x/y/z in place. Returns true if grounded. */
bool game_player_collide (float *cam_x, float *cam_y, float *cam_z,
                           float *vert_vel, float *horiz_vx, float *horiz_vz);

/* Move the player with step-up support. Call before game_player_collide.
 * forward_x/forward_z should be the desired movement direction (normalized).
 * speed is the movement speed in blocks per second. */
void game_move_player (float *cam_x, float *cam_y, float *cam_z,
                        float forward_x, float forward_z, float speed);

/* Rigidbody-voxel collision: push dynamic rigidbodies out of solid blocks.
 * Also transfers momentum to rigidbodies when player collides. */
void game_rigidbody_voxel_collide (void);

/* Get the break progress for a specific block. Returns -1 if not being broken, 0-1 otherwise. */
float game_get_break_progress (int x, int y, int z);

#endif
