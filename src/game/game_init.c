/* PotatoWorld v2 — Game Init Implementation
 * World generation, block interaction, player/world collision.
 * Additive layer on MPE v14S. No engine physics code modified. */

#include "game_init.h"
#include "block.h"
#include "block_render.h"
#include "world.h"
#include "raycast.h"
#include "../core/rigidbody.h"
#include <stdio.h>
#include <math.h>

game_world pw_world;
int pw_selected_block = BLOCK_GRASS;
int pw_block_type_count = 0;

void game_init (void) {
    block_registry_init ();
    pw_block_type_count = block_type_count ();

    world_init (&pw_world, 42);
    world_generate_terrain (&pw_world);

    block_render_init ();

    printf ("[GAME] PotatoWorld initialized: %d block types, world %dx%dx%d blocks\n",
            pw_block_type_count,
            WORLD_CHUNKS_X * CHUNK_SIZE,
            WORLD_CHUNKS_Y * CHUNK_SIZE,
            WORLD_CHUNKS_Z * CHUNK_SIZE);
    printf ("[GAME] Selected block: %s\n", block_type_get (pw_selected_block)->name);
}

void game_update (float delta_time, float cam_x, float cam_y, float cam_z,
                  float dir_x, float dir_y, float dir_z) {
    (void) delta_time;
    (void) cam_x; (void) cam_y; (void) cam_z;
    (void) dir_x; (void) dir_y; (void) dir_z;
}

/* ---- Player-voxel collision (ported from old PotatoWorld) ---- */
/* Test one sphere against all solid blocks near it. Push position out, zero velocities as needed. */
static bool sphere_vs_world (float sx, float sy, float sz, float radius,
                             float *pos_x, float *pos_y, float *pos_z,
                             float *vert_vel, float *horiz_vx, float *horiz_vz) {
    bool grounded = false;
    int bx_min = (int) floorf (sx - radius);
    int bx_max = (int) floorf (sx + radius);
    int by_min = (int) floorf (sy - radius);
    int by_max = (int) floorf (sy + radius);
    int bz_min = (int) floorf (sz - radius);
    int bz_max = (int) floorf (sz + radius);

    for (int bx = bx_min; bx <= bx_max; bx++) {
        for (int by = by_min; by <= by_max; by++) {
            for (int bz = bz_min; bz <= bz_max; bz++) {
                int bid = world_get (&pw_world, bx, by, bz);
                const block_type *bt = block_type_get (bid);
                if (!bt->solid) {continue;}

                /* Voxel AABB: [bx, bx+1] x [by, by+1] x [bz, bz+1] */
                float closest_x = sx;
                float closest_y = sy;
                float closest_z = sz;
                if (closest_x < (float) bx) {closest_x = (float) bx;}
                else if (closest_x > (float) bx + 1.0f) {closest_x = (float) bx + 1.0f;}
                if (closest_y < (float) by) {closest_y = (float) by;}
                else if (closest_y > (float) by + 1.0f) {closest_y = (float) by + 1.0f;}
                if (closest_z < (float) bz) {closest_z = (float) bz;}
                else if (closest_z > (float) bz + 1.0f) {closest_z = (float) bz + 1.0f;}

                float dx = sx - closest_x;
                float dy = sy - closest_y;
                float dz = sz - closest_z;
                float dist_sq = dx * dx + dy * dy + dz * dz;

                if (dist_sq > radius * radius) {continue;}
                if (dist_sq < 0.000001f) {continue;}

                float dist = sqrtf (dist_sq);
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;
                float penetration = radius - dist;

                /* Push sphere center out of the voxel */
                sx += nx * penetration;
                sy += ny * penetration;
                sz += nz * penetration;

                /* Also push the camera position directly */
                *pos_x += nx * penetration;
                *pos_y += ny * penetration;
                *pos_z += nz * penetration;

                /* Vertical response */
                if (ny > 0.5f && *vert_vel < 0.0f) {
                    *vert_vel = 0.0f;
                    grounded = true;
                } else if (ny < -0.5f && *vert_vel > 0.0f) {
                    *vert_vel = 0.0f;
                }

                /* Horizontal wall sliding */
                if (fabsf (nx) > fabsf (ny) || fabsf (nz) > fabsf (ny)) {
                    float vdotn = (*horiz_vx) * (-nx) + (*horiz_vz) * (-nz);
                    if (vdotn < 0.0f) {
                        *horiz_vx -= vdotn * (-nx);
                        *horiz_vz -= vdotn * (-nz);
                    }
                }
            }
        }
    }
    return grounded;
}

bool game_player_collide (float *cam_x, float *cam_y, float *cam_z,
                          float *vert_vel, float *horiz_vx, float *horiz_vz) {
    const float player_radius = 0.4f;
    const float offsets [3] = {-1.4f, -0.8f, -0.2f};
    bool grounded = false;

    for (int i = 0; i < 3; i++) {
        float sx = *cam_x;
        float sy = *cam_y + offsets [i];
        float sz = *cam_z;

        if (sphere_vs_world (sx, sy, sz, player_radius,
                             cam_x, cam_y, cam_z,
                             vert_vel, horiz_vx, horiz_vz)) {
            grounded = true;
        }
    }
    return grounded;
}

void game_on_left_click (float cam_x, float cam_y, float cam_z,
                         float dir_x, float dir_y, float dir_z) {
    raycast_result r = raycast_voxel (&pw_world,
                                      cam_x, cam_y, cam_z,
                                      dir_x, dir_y, dir_z,
                                      8.0f);
    if (r.hit) {
        world_set (&pw_world, r.block_x, r.block_y, r.block_z, BLOCK_AIR);
        printf ("[GAME] Removed block at (%d, %d, %d)\n", r.block_x, r.block_y, r.block_z);
    }
}

void game_on_right_click (float cam_x, float cam_y, float cam_z,
                          float dir_x, float dir_y, float dir_z) {
    raycast_result r = raycast_voxel (&pw_world,
                                      cam_x, cam_y, cam_z,
                                      dir_x, dir_y, dir_z,
                                      8.0f);
    if (r.hit) {
        int px = r.block_x + r.normal_x;
        int py = r.block_y + r.normal_y;
        int pz = r.block_z + r.normal_z;

        /* Don't place inside the player (simple AABB check) */
        float bx = (float) px + 0.5f;
        float by = (float) py + 0.5f;
        float bz = (float) pz + 0.5f;
        float dx = cam_x - bx;
        float dy = cam_y - by;
        float dz = cam_z - bz;
        if (dx * dx + dy * dy + dz * dz < 1.0f) {return;}

        world_set (&pw_world, px, py, pz, pw_selected_block);
        printf ("[GAME] Placed %s at (%d, %d, %d)\n",
                block_type_get (pw_selected_block)->name, px, py, pz);
    }
}

void game_scroll_block (int direction) {
    pw_selected_block += direction;
    if (pw_selected_block >= pw_block_type_count) {pw_selected_block = 1;}
    if (pw_selected_block < 1) {pw_selected_block = pw_block_type_count - 1;}
    printf ("[GAME] Selected block: %s (id=%d)\n",
            block_type_get (pw_selected_block)->name, pw_selected_block);
}

/* ---- Rigidbody-vs-voxel collision ---- */
/* Extern from the engine — the object pool */
extern rigidbody *obj_per_scene;
extern int object_count;

void game_rigidbody_voxel_collide (void) {
    for (int i = 0; i < object_count; i++) {
        rigidbody *rb = &obj_per_scene [i];
        if (rb->static_state) {continue;}
        if (rb->is_sleeping) {continue;}

        float radius = 0.0f;
        if (rb->type == object_sphere) {
            radius = rb->radius;
        } else if (rb->type == object_cube) {
            /* Use the largest half-extent as effective collision radius */
            float max_he = rb->half_extensions.x;
            if (rb->half_extensions.y > max_he) {max_he = rb->half_extensions.y;}
            if (rb->half_extensions.z > max_he) {max_he = rb->half_extensions.z;}
            radius = max_he;
        } else {
            continue;
        }

        /* Find all solid voxels within range */
        int bx_min = (int) floorf (rb->position.x - radius - 0.5f);
        int bx_max = (int) floorf (rb->position.x + radius + 0.5f);
        int by_min = (int) floorf (rb->position.y - radius - 0.5f);
        int by_max = (int) floorf (rb->position.y + radius + 0.5f);
        int bz_min = (int) floorf (rb->position.z - radius - 0.5f);
        int bz_max = (int) floorf (rb->position.z + radius + 0.5f);

        bool touched = false;

        for (int bx = bx_min; bx <= bx_max; bx++) {
            for (int by = by_min; by <= by_max; by++) {
                for (int bz = bz_min; bz <= bz_max; bz++) {
                    int bid = world_get (&pw_world, bx, by, bz);
                    const block_type *bt = block_type_get (bid);
                    if (!bt->solid) {continue;}

                    /* Closest point on voxel AABB to sphere center */
                    float cx = rb->position.x;
                    float cy = rb->position.y;
                    float cz = rb->position.z;
                    if (cx < (float) bx) {cx = (float) bx;}
                    else if (cx > (float) bx + 1.0f) {cx = (float) bx + 1.0f;}
                    if (cy < (float) by) {cy = (float) by;}
                    else if (cy > (float) by + 1.0f) {cy = (float) by + 1.0f;}
                    if (cz < (float) bz) {cz = (float) bz;}
                    else if (cz > (float) bz + 1.0f) {cz = (float) bz + 1.0f;}

                    float dx = rb->position.x - cx;
                    float dy = rb->position.y - cy;
                    float dz = rb->position.z - cz;
                    float dist_sq = dx * dx + dy * dy + dz * dz;

                    if (dist_sq > radius * radius) {continue;}
                    if (dist_sq < 0.000001f) {
                        /* Center is inside the voxel — push out along velocity or up */
                        float push_x = 0.0f, push_y = 0.5f, push_z = 0.0f;
                        if (fabsf (rb->velocity.x) > fabsf (rb->velocity.z)) {
                            push_x = (rb->velocity.x > 0) ? -0.5f : 0.5f;
                            push_y = 0.0f;
                        } else if (fabsf (rb->velocity.z) > 0.01f) {
                            push_z = (rb->velocity.z > 0) ? -0.5f : 0.5f;
                            push_y = 0.0f;
                        }
                        rb->position.x += push_x;
                        rb->position.y += push_y;
                        rb->position.z += push_z;
                        touched = true;
                        continue;
                    }

                    float dist = sqrtf (dist_sq);
                    float nx = dx / dist;
                    float ny = dy / dist;
                    float nz = dz / dist;
                    float penetration = radius - dist;

                    rb->position.x += nx * penetration;
                    rb->position.y += ny * penetration;
                    rb->position.z += nz * penetration;

                    /* Cancel velocity into the surface */
                    float vel_dot_n = rb->velocity.x * nx + rb->velocity.y * ny + rb->velocity.z * nz;
                    if (vel_dot_n < 0.0f) {
                        rb->velocity.x -= vel_dot_n * nx;
                        rb->velocity.y -= vel_dot_n * ny;
                        rb->velocity.z -= vel_dot_n * nz;
                    }

                    /* Cancel angular velocity contribution at contact */
                    if (ny > 0.5f && rb->velocity.y < 0.0f) {
                        rb->velocity.y = 0.0f;
                    }

                    touched = true;
                }
            }
        }

        if (touched && rb->is_sleeping) {
            rigidbody_wake (rb);
        }
    }
}
