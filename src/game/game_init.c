/* v10S — Game Init Implementation
 * Minecraft-like player (0.6x1.8), hold-to-break, OBB voxel collisions.
 * Additive layer on v10S. No engine physics code modified. */

#include "../mpe_engine.h"
#include "game_init.h"
#include "block.h"
#include "block_render.h"
#include "world.h"
#include "raycast.h"
#include "player.h"
#include "item_registry.h"
#include "../core/rigidbody.h"
#include "../ui_input/camera.h"
#include "../scene/scene_init.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

game_world pw_world;
int pw_selected_block = BLOCK_GRASS;
int pw_block_type_count = 0;

typedef struct { int x, y, z; float progress; } break_entry_t;
static break_entry_t break_entries [1024];
static int break_entry_count = 0;

/* hold-to-break state */
static int last_break_x = 1000000, last_break_y = 1000000, last_break_z = 1000000;

static int find_break_entry (int x, int y, int z) {
    for (int i = 0; i < break_entry_count; i++) {
        if (break_entries [i].x == x && break_entries [i].y == y && break_entries [i].z == z) {return i;}
    }
    return -1;
}

static int block_to_item (int block_id) {
    switch(block_id){
        case BLOCK_GRASS: return ITEM_DIRT;
        case BLOCK_DIRT: return ITEM_DIRT;
        case BLOCK_STONE: return ITEM_STONE;
        case BLOCK_OAK_LOG: return ITEM_OAK_LOG;
        case BLOCK_SAND: return ITEM_SAND;
        case BLOCK_OAK_PLANKS: return ITEM_OAK_PLANKS;
        case BLOCK_COBBLESTONE: return ITEM_COBBLESTONE;
        case BLOCK_BRICKS: return ITEM_COBBLESTONE; // fallback
        case BLOCK_GLASS: return ITEM_COBBLESTONE;
        case BLOCK_OAK_LEAVES: return ITEM_OAK_SAPLING;
        case BLOCK_IRON_ORE: return ITEM_COBBLESTONE;
        case BLOCK_GOLD_ORE: return ITEM_COBBLESTONE;
        case BLOCK_DIAMOND_ORE: return ITEM_DIAMOND;
        case BLOCK_COAL_ORE: return ITEM_COAL;
        case BLOCK_GRAVEL: return ITEM_COBBLESTONE;
        case BLOCK_BEDROCK: return ITEM_STONE;
        default: {
            // try to find item whose block_id matches
            for(int i=0;i<item_type_count();i++){ const item_type *it=item_type_get(i); if(it && it->block_id==block_id) return i; }
            return ITEM_COBBLESTONE;
        }
    }
}

static void add_break_progress (int x, int y, int z, float amount) {
    int bid = world_get(&pw_world,x,y,z);
    const block_type *bt = block_type_get(bid);
    if (bt->hardness < 0.0f) return; // bedrock unbreakable
    int idx = find_break_entry (x, y, z);
    if (idx >= 0) {
        break_entries [idx].progress += amount;
        if (break_entries [idx].progress >= 1.0f) {
            // drop to inventory in survival
            if (g_player.stats.game_mode != GAME_MODE_CREATIVE) {
                int item = block_to_item(bid);
                inventory_add_item(item, 1, 0);
            }
            world_set (&pw_world, x, y, z, BLOCK_AIR);
            for (int i = idx; i < break_entry_count - 1; i++) {
                break_entries [i] = break_entries [i + 1];
            }
            break_entry_count--;
            last_break_x = 1000000; last_break_y = 1000000; last_break_z = 1000000;
        }
    } else if (break_entry_count < 1024) {
        break_entries [break_entry_count].x = x;
        break_entries [break_entry_count].y = y;
        break_entries [break_entry_count].z = z;
        break_entries [break_entry_count].progress = amount;
        break_entry_count++;
    }
}

static bool raycast_forward (float ox, float oy, float oz, float dx, float dy, float dz, float max_dist, int *hit_x, int *hit_y, int *hit_z) {
    raycast_result r = raycast_voxel (&pw_world, ox, oy, oz, dx, dy, dz, max_dist);
    if (r.hit) {*hit_x = r.block_x; *hit_y = r.block_y; *hit_z = r.block_z; return true;}
    return false;
}

extern camera main_camera_fov;

void game_init (void) {
    block_registry_init ();
    item_registry_init();
    player_init();
    pw_block_type_count = block_type_count ();

    world_init (&pw_world, 42);
    world_generate_terrain (&pw_world);

    block_render_init ();

    // Find safe spawn height at 64,64
    int spawn_x = 64, spawn_z = 64;
    int spawn_y = 20;
    for (int y = WORLD_CHUNKS_Y*CHUNK_SIZE-1; y >= -3; y--) {
        if (world_get(&pw_world, spawn_x, y, spawn_z) != BLOCK_AIR) { spawn_y = y + 3; break; }
    }
    main_camera_fov.position.x = (float)spawn_x + 0.5f;
    main_camera_fov.position.z = (float)spawn_z + 0.5f;
    main_camera_fov.position.y = (float)spawn_y + PLAYER_HEIGHT;
    main_camera_fov.vertical_velocity = 0;
    main_camera_fov.horizontal_velocity = (vector3){0,0,0};

    // sync selected block with hotbar
    int itm = player_get_item_in_hand();
    const item_type *it = item_type_get(itm);
    if (it && it->placeable && it->block_id>=0) pw_selected_block = it->block_id;

    // Hardcoded object renders for demo (visible in both Creative/Survival)
    {
        int hc1 = scene_add_object(0.5f, 1.0f, (vector3){68, spawn_y+1, 68});
        if (hc1>=0){ obj_per_scene[hc1].colour = (vector3){1,0.25,0.25}; obj_per_scene[hc1].restitution=0.55f; obj_per_scene[hc1].friction_static=0.6f; }
        int hc2 = scene_add_cube((vector3){70, spawn_y+1, 68}, (vector3){0.5f,0.5f,0.5f}, 2.0f);
        if (hc2>=0){ obj_per_scene[hc2].colour = (vector3){0.25,1,0.25}; obj_per_scene[hc2].friction_static=0.8f; }
        int hc3 = scene_add_cube((vector3){72, spawn_y+1, 68}, (vector3){0.5f,0.5f,0.5f}, 1.5f);
        if (hc3>=0){ obj_per_scene[hc3].colour = (vector3){0.25,0.25,1}; obj_per_scene[hc3].friction_static=0.9f; }
        // floating glass showcase
        int hc4 = scene_add_cube((vector3){69, spawn_y+4, 70}, (vector3){0.4f,0.4f,0.4f}, 1.0f);
        if (hc4>=0){ obj_per_scene[hc4].colour = (vector3){0.82f,0.92f,1.0f}; }
    }

    printf ("[GAME] v10S initialized: %d block types, world %dx%dx%d blocks\n",
            pw_block_type_count,
            WORLD_CHUNKS_X * CHUNK_SIZE,
            WORLD_CHUNKS_Y * CHUNK_SIZE,
            WORLD_CHUNKS_Z * CHUNK_SIZE);
    printf ("[GAME] Selected block: %s | Spawn at %d %d %d | Mode %s\n", block_type_get (pw_selected_block)->name, spawn_x, spawn_y, spawn_z, g_player.stats.game_mode==GAME_MODE_CREATIVE?"Creative":"Survival");
}

void game_update (float delta_time, float cam_x, float cam_y, float cam_z,
                   float dir_x, float dir_y, float dir_z) {
    (void) delta_time;
    (void) cam_x; (void) cam_y; (void) cam_z;
    (void) dir_x; (void) dir_y; (void) dir_z;
}

/* ---- Minecraft-like AABB-vs-world collision ----
 * Player AABB: 0.6 wide (x/z) × 1.8 tall (y). Position is head (camera) at top.
 * Feet at y - PLAYER_HEIGHT, head at y.
 * Iterative multi-axis resolution: resolve smallest penetration per iteration,
 * recompute and repeat up to 4 times. Y resolved first for grounded stability.
 */
static bool aabb_vs_world (float *pos_x, float *pos_y, float *pos_z) {
    bool grounded = false;
    bool hit_ceiling = false;

    for (int iter=0; iter<4; iter++) {
        float min_x = *pos_x - PLAYER_HALF_W;
        float max_x = *pos_x + PLAYER_HALF_W;
        float min_y = *pos_y - PLAYER_HEIGHT;
        float max_y = *pos_y;
        float min_z = *pos_z - PLAYER_HALF_W;
        float max_z = *pos_z + PLAYER_HALF_W;

        int bx_min = (int) floorf (min_x);
        int bx_max = (int) floorf (max_x);
        int bz_min = (int) floorf (min_z);
        int bz_max = (int) floorf (max_z);
        int by_min = (int) floorf (min_y) - 1;
        int by_max = (int) floorf (max_y) + 1;

        float best_pen_x=0, best_pen_y=0, best_pen_z=0;
        float best_overlap = 1e9f;
        int best_axis = -1;

        for (int bx = bx_min; bx <= bx_max; bx++) {
            for (int by = by_min; by <= by_max; by++) {
                for (int bz = bz_min; bz <= bz_max; bz++) {
                    int bid = world_get (&pw_world, bx, by, bz);
                    const block_type *bt = block_type_get (bid);
                    if (!bt->solid) continue;

                    float ox_min = (float) bx;
                    float ox_max = (float) bx + 1.0f;
                    float oy_min = (float) by;
                    float oy_max = (float) by + 1.0f;
                    float oz_min = (float) bz;
                    float oz_max = (float) bz + 1.0f;

                    float overlap_x = fminf(max_x, ox_max) - fmaxf(min_x, ox_min);
                    float overlap_y = fminf(max_y, oy_max) - fmaxf(min_y, oy_min);
                    float overlap_z = fminf(max_z, oz_max) - fmaxf(min_z, oz_min);

                    if (overlap_x <= 0 || overlap_y <= 0 || overlap_z <= 0) continue;

                    // Find smallest penetration axis
                    float overlap_min = overlap_x;
                    int axis = 0;
                    if (overlap_y < overlap_min) {overlap_min = overlap_y; axis = 1;}
                    if (overlap_z < overlap_min) {overlap_min = overlap_z; axis = 2;}

                    // Minecraft-style Y priority: if block is directly below/above feet, prefer Y
                    float floor_dist = fabsf(oy_max - min_y);
                    float ceil_dist = fabsf(oy_min - max_y);
                    if (axis != 1 && overlap_y < 0.6f) {
                        if (floor_dist < 0.15f || ceil_dist < 0.15f) {
                            axis = 1; overlap_min = overlap_y;
                        }
                    }

                    if (overlap_min < best_overlap) {
                        best_overlap = overlap_min;
                        best_axis = axis;
                        if (axis == 0) {
                            best_pen_x = (min_x < ox_min) ? -overlap_x : overlap_x;
                            // push away from block center
                            if (*pos_x < (ox_min+ox_max)*0.5f) best_pen_x = -overlap_x;
                            else best_pen_x = overlap_x;
                            best_pen_y = 0; best_pen_z = 0;
                        } else if (axis == 1) {
                            best_pen_x = 0;
                            if (min_y < oy_min) best_pen_y = -overlap_y;
                            else best_pen_y = overlap_y;
                            best_pen_z = 0;
                            if (best_pen_y > 0) grounded = true;
                            else hit_ceiling = true;
                        } else {
                            best_pen_x = 0; best_pen_y = 0;
                            if (min_z < oz_min) best_pen_z = -overlap_z;
                            else best_pen_z = overlap_z;
                            if (*pos_z < (oz_min+oz_max)*0.5f) best_pen_z = -overlap_z;
                            else best_pen_z = overlap_z;
                        }
                    }
                }
            }
        }

        if (best_axis < 0) break; // no overlap

        // Apply smallest penetration
        if (best_axis == 0) *pos_x += best_pen_x;
        else if (best_axis == 1) *pos_y += best_pen_y;
        else *pos_z += best_pen_z;

        // If we resolved Y positively, we are grounded; continue to resolve XZ next iter
        if (best_axis == 1 && best_pen_y > 0) grounded = true;
        if (best_axis == 1 && best_pen_y < 0) hit_ceiling = true;

        // If penetration was tiny (<0.001) break
        if (fabsf(best_pen_x)+fabsf(best_pen_y)+fabsf(best_pen_z) < 0.001f) break;
    }

    // Additional floor snap: if feet within 0.05 of solid block top, snap to top
    {
        float min_y = *pos_y - PLAYER_HEIGHT;
        int bx_min = (int) floorf (*pos_x - PLAYER_HALF_W);
        int bx_max = (int) floorf (*pos_x + PLAYER_HALF_W);
        int bz_min = (int) floorf (*pos_z - PLAYER_HALF_W);
        int bz_max = (int) floorf (*pos_z + PLAYER_HALF_W);
        int by = (int) floorf (min_y - 0.02f);
        for (int bx=bx_min; bx<=bx_max; bx++) for (int bz=bz_min; bz<=bz_max; bz++) {
            int bid = world_get(&pw_world,bx,by,bz);
            if (!block_type_get(bid)->solid) continue;
            float top = (float)by+1.0f;
            float dist = min_y - top;
            if (dist >= -0.05f && dist <= 0.05f) {
                *pos_y = top + PLAYER_HEIGHT;
                grounded = true;
            }
        }
    }

    return grounded && !hit_ceiling;
}

static bool attempt_step_up (float *pos_x, float *pos_y, float *pos_z, float move_x, float move_z) {
    float new_x = *pos_x + move_x;
    float new_z = *pos_z + move_z;

    // foot position
    float foot_y = *pos_y - PLAYER_HEIGHT + 0.15f;
    float dir_x = (move_x != 0) ? (move_x > 0 ? 1.0f : -1.0f) : 0.0f;
    float dir_z = (move_z != 0) ? (move_z > 0 ? 1.0f : -1.0f) : 0.0f;
    float len = sqrtf(dir_x*dir_x+dir_z*dir_z);
    if (len < 0.001f) return false;
    dir_x/=len; dir_z/=len;

    int hit_x, hit_y, hit_z;
    if (raycast_forward (*pos_x, foot_y, *pos_z, dir_x, 0.0f, dir_z, 0.7f, &hit_x, &hit_y, &hit_z)) {
        if (world_get (&pw_world, hit_x, hit_y, hit_z) != BLOCK_AIR) {
            // check step height free: need current block+1 and +2 free
            if (world_get (&pw_world, hit_x, hit_y + 1, hit_z) == BLOCK_AIR &&
                world_get (&pw_world, hit_x, hit_y + 2, hit_z) == BLOCK_AIR) {
                // ensure space at new position feet+step
                int check_x = (int)floorf(new_x);
                int check_z = (int)floorf(new_z);
                int base_y = (int)floorf(*pos_y - PLAYER_HEIGHT + STEP_HEIGHT);
                if (world_get(&pw_world, check_x, base_y, check_z)==BLOCK_AIR &&
                    world_get(&pw_world, check_x, base_y+1, check_z)==BLOCK_AIR) {
                    *pos_y += STEP_HEIGHT;
                    *pos_x = new_x;
                    *pos_z = new_z;
                    return true;
                }
            }
        }
    }
    return false;
}

bool game_player_collide (float *cam_x, float *cam_y, float *cam_z,
                           float *vert_vel, float *horiz_vx, float *horiz_vz) {
    // apply damping to horizontal velocity (already done in simulation, but ensure)
    (void) horiz_vx; (void) horiz_vz;
    bool grounded = aabb_vs_world (cam_x, cam_y, cam_z);

    float min_y = *cam_y - PLAYER_HEIGHT;
    // world bottom safety: keep above bedrock
    if (min_y < -2.5f) {
        *cam_y = -2.5f + PLAYER_HEIGHT;
        *vert_vel = 0.0f;
        grounded = true;
    }
    if (*cam_x < -250.0f) {*cam_x = -250.0f; if (horiz_vx) *horiz_vx=0;}
    if (*cam_x > 250.0f) {*cam_x = 250.0f; if (horiz_vx) *horiz_vx=0;}
    if (*cam_z < -250.0f) {*cam_z = -250.0f; if (horiz_vz) *horiz_vz=0;}
    if (*cam_z > 250.0f) {*cam_z = 250.0f; if (horiz_vz) *horiz_vz=0;}

    return grounded;
}

void game_on_left_click (float cam_x, float cam_y, float cam_z,
                          float dir_x, float dir_y, float dir_z) {
    raycast_result r = raycast_voxel (&pw_world, cam_x, cam_y, cam_z, dir_x, dir_y, dir_z, BREAK_MAX_DISTANCE);
    if (!r.hit) return;
    int bid = world_get (&pw_world, r.block_x, r.block_y, r.block_z);
    if (bid == BLOCK_AIR) return;
    const block_type *bt = block_type_get (bid);
    if (!bt->solid) return;
    if (bt->hardness == -1.0f) return; // bedrock
    if (bt->hardness <= 0.2f) {
        if (g_player.stats.game_mode != GAME_MODE_CREATIVE) {
            int item = block_to_item(bid);
            inventory_add_item(item,1,0);
        }
        world_set(&pw_world, r.block_x, r.block_y, r.block_z, BLOCK_AIR);
        printf("[GAME] Broke %s at (%d,%d,%d) instant\n", bt->name, r.block_x, r.block_y, r.block_z);
    }
}

void game_tick_break (float dt, float cam_x, float cam_y, float cam_z,
                      float dir_x, float dir_y, float dir_z, bool is_holding) {
    if (!is_holding) {
        // always reset to nil after you stop pressing (as requested)
        if (break_entry_count > 0) {
            break_entry_count = 0;
            last_break_x = 1000000; last_break_y = 1000000; last_break_z = 1000000;
        }
        return;
    }
    raycast_result r = raycast_voxel (&pw_world, cam_x, cam_y, cam_z, dir_x, dir_y, dir_z, BREAK_MAX_DISTANCE);
    if (!r.hit) {
        // looking at air — don't reset all, but clear last target timer?
        return;
    }
    int bid = world_get (&pw_world, r.block_x, r.block_y, r.block_z);
    const block_type *bt = block_type_get (bid);
    if (bid == BLOCK_AIR || !bt->solid) return;

    // if target changed, reset old block's progress (Minecraft-like)
    if (r.block_x != last_break_x || r.block_y != last_break_y || r.block_z != last_break_z) {
        int old_idx = find_break_entry(last_break_x, last_break_y, last_break_z);
        if (old_idx >= 0) {
            for (int i = old_idx; i < break_entry_count - 1; i++) break_entries[i] = break_entries[i+1];
            break_entry_count--;
        }
        last_break_x = r.block_x; last_break_y = r.block_y; last_break_z = r.block_z;
    }

    float break_time = bt->hardness * 1.5f; // Minecraft: hardness*1.5 seconds with fist
    if (bt->hardness <= 0.0f) break_time = 0.05f;
    if (break_time < 0.2f) break_time = 0.2f; // minimum hold for soft blocks except instant above

    if (bt->hardness == -1.0f) return;
    // leaves/glass handled via instant above, but also allow hold
    if (bt->hardness <= 0.2f) {
        if (g_player.stats.game_mode != GAME_MODE_CREATIVE) {
            int item = block_to_item(bid);
            inventory_add_item(item,1,0);
        }
        world_set(&pw_world, r.block_x, r.block_y, r.block_z, BLOCK_AIR);
        printf("[GAME] Broke %s at (%d,%d,%d)\n", bt->name, r.block_x, r.block_y, r.block_z);
        return;
    }

    float add = dt / break_time;
    add_break_progress(r.block_x, r.block_y, r.block_z, add);
    int bidx = find_break_entry(r.block_x, r.block_y, r.block_z);
    float prog = (bidx>=0)? break_entries[bidx].progress : 1.0f;
    if (bidx>=0 && fmodf(prog*10.0f,1.0f) < 0.1f) {
        // occasional print
        //printf("[GAME] Breaking %s %d %d %d %.0f%%\n", bt->name, r.block_x, r.block_y, r.block_z, prog*100);
    }
    if (prog >= 1.0f || bidx<0) {
        // broken
        printf("[GAME] Broke %s at (%d,%d,%d)\n", bt->name, r.block_x, r.block_y, r.block_z);
    }
}

void game_tick_falling (void) {
    world_tick_falling(&pw_world);
}

void game_move_player (float *cam_x, float *cam_y, float *cam_z,
                        float forward_x, float forward_z, float speed) {
    float target_x = *cam_x + forward_x * speed;
    float target_z = *cam_z + forward_z * speed;
    float dx = target_x - *cam_x;
    float dz = target_z - *cam_z;

    if (!attempt_step_up (cam_x, cam_y, cam_z, dx, dz)) {
        *cam_x = target_x;
        *cam_z = target_z;
    }
}

float game_get_break_progress (int x, int y, int z) {
    int idx = find_break_entry (x, y, z);
    if (idx >= 0) {return break_entries [idx].progress;}
    return -1.0f;
}

void game_on_right_click (float cam_x, float cam_y, float cam_z,
                            float dir_x, float dir_y, float dir_z) {
    // Eating: if holding food, eat on right-click (even in air)
    {
        int hand = player_get_item_in_hand();
        const item_type *it = item_type_get(hand);
        if (it && it->is_food) {
            // only eat if not full (or always in creative)
            if (g_player.stats.hunger < g_player.stats.max_hunger || g_player.stats.health < g_player.stats.max_health || g_player.stats.game_mode==GAME_MODE_CREATIVE) {
                // heal hunger/saturation
                float new_hunger = g_player.stats.hunger + it->food_value;
                if (new_hunger > g_player.stats.max_hunger) new_hunger = g_player.stats.max_hunger;
                g_player.stats.hunger = new_hunger;
                g_player.stats.saturation += it->saturation;
                if (g_player.stats.saturation > g_player.stats.hunger) g_player.stats.saturation = g_player.stats.hunger;
                player_heal(it->food_value * 0.5f);
                // consume one from hand slot
                if (g_player.stats.game_mode != GAME_MODE_CREATIVE) {
                    int slot = g_player.inventory.selected_hotbar_slot;
                    item_stack_t *st = inventory_get_slot(slot);
                    if (st && st->item_id==hand && st->count>0) {
                        st->count--; if(st->count<=0){st->item_id=ITEM_AIR; st->damage=0;}
                    } else {
                        inventory_remove_item(hand,1);
                    }
                }
                printf("[GAME] Ate %s (hunger %.0f saturation %.1f)\n", it->name, g_player.stats.hunger, g_player.stats.saturation);
                // sync selected block after consume
                int nhand = player_get_item_in_hand();
                const item_type *nit = item_type_get(nhand);
                if (nit && nit->placeable && nit->block_id>=0) pw_selected_block = nit->block_id;
                else if (g_player.stats.game_mode!=GAME_MODE_CREATIVE) {
                    // find next placeable in hotbar
                    for(int i=0;i<9;i++){ item_stack_t *s=inventory_get_slot(i); const item_type *t=item_type_get(s? s->item_id:0); if(t&&t->placeable){ pw_selected_block=t->block_id; break; } }
                }
                return; // don't place block when eating
            }
        }
    }
    raycast_result r = raycast_voxel (&pw_world,
                                          cam_x, cam_y, cam_z,
                                          dir_x, dir_y, dir_z,
                                          8.0f);
    if (r.hit) {
        int px = r.block_x + r.normal_x;
        int py = r.block_y + r.normal_y;
        int pz = r.block_z + r.normal_z;

        float bx = (float) px + 0.5f;
        float by = (float) py + 0.5f;
        float bz = (float) pz + 0.5f;
        float dx = cam_x - bx;
        float dy = cam_y - by;
        float dz = cam_z - bz;
        if (dx * dx + dy * dy + dz * dz < 1.0f) {return;}

        // prevent placing inside player AABB
        float p_min_x = cam_x - PLAYER_HALF_W, p_max_x = cam_x + PLAYER_HALF_W;
        float p_min_y = cam_y - PLAYER_HEIGHT, p_max_y = cam_y;
        float p_min_z = cam_z - PLAYER_HALF_W, p_max_z = cam_z + PLAYER_HALF_W;
        if (px+1 > p_min_x && px < p_max_x && py+1 > p_min_y && py < p_max_y && pz+1 > p_min_z && pz < p_max_z) return;

        if (py < -3) {return;}

        // check target empty
        if (world_get(&pw_world, px,py,pz) != BLOCK_AIR) return;

        // survival: need item
        if (g_player.stats.game_mode != GAME_MODE_CREATIVE) {
            // try to consume from inventory hotbar
            int slot = g_player.inventory.selected_hotbar_slot;
            item_stack_t *st = inventory_get_slot(slot);
            const item_type *it = NULL;
            if (st) it = item_type_get(st->item_id);
            bool has = false;
            if (st && st->item_id != ITEM_AIR && st->count>0 && it && it->block_id == pw_selected_block) has = true;
            // also allow any slot that has this block if hotbar empty (convenience)
            if (!has) {
                // search inventory for requested block
                for(int i=0;i<36;i++){ item_stack_t *s=inventory_get_slot(i); if(!s) continue; const item_type *t=item_type_get(s->item_id); if(t && t->block_id==pw_selected_block && s->count>0){ st=s; has=true; break; } }
            }
            if (!has) { printf("[GAME] No %s in inventory\n", block_type_get(pw_selected_block)->name); return; }
            // consume one
            st->count--; if(st->count<=0){st->item_id=ITEM_AIR; st->damage=0;}
            // sync selected if hotbar consumed
            int hand = player_get_item_in_hand();
            const item_type *hit = item_type_get(hand);
            if (hit && hit->block_id>=0) pw_selected_block = hit->block_id;
        }

        world_set (&pw_world, px, py, pz, pw_selected_block);
        printf ("[GAME] Placed %s at (%d, %d, %d)\n",
                block_type_get (pw_selected_block)->name, px, py, pz);
    }
}

void game_scroll_block (int direction) {
    pw_selected_block += direction;
    if (pw_selected_block >= pw_block_type_count) {pw_selected_block = 1;}
    if (pw_selected_block < 1) {pw_selected_block = pw_block_type_count - 1;}
    // skip AIR if we wrap
    if (pw_selected_block == BLOCK_AIR) pw_selected_block = (direction>0?1:pw_block_type_count-1);
    printf ("[GAME] Selected block: %s (id=%d)\n",
            block_type_get (pw_selected_block)->name, pw_selected_block);
}

/* ---- Rigidbody-vs-voxel collision ----
 * Proper OBB vs voxel AABB via closest-point, not sphere approx.
 * Uses cube half_extents + cached_axes for oriented cubes.
 */
extern rigidbody *obj_per_scene;
extern int object_count;

static bool obb_voxel_collide (rigidbody *rb, int bx, int by, int bz, vector3 *out_normal, float *out_pen) {
    // voxel AABB center 0.5, half 0.5
    float vox_cx = (float)bx + 0.5f;
    float vox_cy = (float)by + 0.5f;
    float vox_cz = (float)bz + 0.5f;

    if (rb->type == object_sphere) {
        float closest_x = fmaxf((float)bx, fminf(rb->position.x, (float)bx+1.0f));
        float closest_y = fmaxf((float)by, fminf(rb->position.y, (float)by+1.0f));
        float closest_z = fmaxf((float)bz, fminf(rb->position.z, (float)bz+1.0f));
        float dx = rb->position.x - closest_x;
        float dy = rb->position.y - closest_y;
        float dz = rb->position.z - closest_z;
        float dist_sq = dx*dx+dy*dy+dz*dz;
        float r = rb->radius;
        if (dist_sq >= r*r) return false;
        float dist = sqrtf(dist_sq);
        if (dist < 0.0001f) {
            // center inside voxel — push up
            *out_normal = (vector3){0,1,0};
            *out_pen = r + 0.5f;
            return true;
        }
        *out_normal = (vector3){dx/dist, dy/dist, dz/dist};
        *out_pen = r - dist;
        return true;
    } else {
        // OBB vs AABB: find closest point on OBB to voxel center, then check voxel
        // Simpler: compute OBB closest to voxel closest point?
        // We do: transform voxel into OBB local space, clamp to half_ext, get closest on OBB
        vector3 rel = {vox_cx - rb->position.x, vox_cy - rb->position.y, vox_cz - rb->position.z};
        // rotate by inverse orientation (transpose of cached axes)
        vector3 *axes = rb->cached_axes;
        // axes are column vectors of rotation matrix; inverse = transpose
        float lx = rel.x*axes[0].x + rel.y*axes[0].y + rel.z*axes[0].z;
        float ly = rel.x*axes[1].x + rel.y*axes[1].y + rel.z*axes[1].z;
        float lz = rel.x*axes[2].x + rel.y*axes[2].y + rel.z*axes[2].z;

        vector3 he = rb->half_extensions;
        float clamped_x = fmaxf(-he.x, fminf(lx, he.x));
        float clamped_y = fmaxf(-he.y, fminf(ly, he.y));
        float clamped_z = fmaxf(-he.z, fminf(lz, he.z));

        // closest point on OBB in world space
        vector3 closest_obb = {
            rb->position.x + axes[0].x*clamped_x + axes[1].x*clamped_y + axes[2].x*clamped_z,
            rb->position.y + axes[0].y*clamped_x + axes[1].y*clamped_y + axes[2].y*clamped_z,
            rb->position.z + axes[0].z*clamped_x + axes[1].z*clamped_y + axes[2].z*clamped_z
        };

        // now closest point on voxel AABB to closest_obb
        float vox_closest_x = fmaxf((float)bx, fminf(closest_obb.x, (float)bx+1.0f));
        float vox_closest_y = fmaxf((float)by, fminf(closest_obb.y, (float)by+1.0f));
        float vox_closest_z = fmaxf((float)bz, fminf(closest_obb.z, (float)bz+1.0f));

        float dx = closest_obb.x - vox_closest_x;
        float dy = closest_obb.y - vox_closest_y;
        float dz = closest_obb.z - vox_closest_z;
        float dist_sq = dx*dx+dy*dy+dz*dz;
        if (dist_sq > 0.000001f) return false; // no penetration if closest_obb outside voxel and separated
        // If dist_sq ==0, OBB touches/intersects voxel
        // Need penetration: compute overlap along normal
        // For OBB inside voxel or touching, we need to push out
        // Find axis of minimum penetration by testing voxel faces against OBB local
        // Fallback: use direction from voxel center to rb center
        float ddx = rb->position.x - vox_cx;
        float ddy = rb->position.y - vox_cy;
        float ddz = rb->position.z - vox_cz;
        // Determine voxel face normal closest to OBB
        // Check overlaps per axis
        float overlap_x = (he.x + 0.5f) - fabsf(lx);
        float overlap_y = (he.y + 0.5f) - fabsf(ly);
        float overlap_z = (he.z + 0.5f) - fabsf(lz);
        if (overlap_x < 0 || overlap_y < 0 || overlap_z < 0) return false;

        // smallest overlap is penetration
        if (overlap_x < overlap_y && overlap_x < overlap_z) {
            float sign = (lx > 0) ? 1.0f : -1.0f;
            *out_normal = (vector3){axes[0].x*sign, axes[0].y*sign, axes[0].z*sign};
            // ensure normal points from voxel to rb
            if ((ddx*out_normal->x + ddy*out_normal->y + ddz*out_normal->z) < 0) {
                out_normal->x = -out_normal->x; out_normal->y = -out_normal->y; out_normal->z = -out_normal->z;
            }
            *out_pen = overlap_x;
        } else if (overlap_y < overlap_z) {
            float sign = (ly > 0) ? 1.0f : -1.0f;
            *out_normal = (vector3){axes[1].x*sign, axes[1].y*sign, axes[1].z*sign};
            if ((ddx*out_normal->x + ddy*out_normal->y + ddz*out_normal->z) < 0) {
                out_normal->x = -out_normal->x; out_normal->y = -out_normal->y; out_normal->z = -out_normal->z;
            }
            *out_pen = overlap_y;
        } else {
            float sign = (lz > 0) ? 1.0f : -1.0f;
            *out_normal = (vector3){axes[2].x*sign, axes[2].y*sign, axes[2].z*sign};
            if ((ddx*out_normal->x + ddy*out_normal->y + ddz*out_normal->z) < 0) {
                out_normal->x = -out_normal->x; out_normal->y = -out_normal->y; out_normal->z = -out_normal->z;
            }
            *out_pen = overlap_z;
        }
        return true;
    }
}

void game_rigidbody_voxel_collide (void) {
    for (int i = 0; i < object_count; i++) {
        rigidbody *rb = &obj_per_scene [i];
        if (rb->static_state) continue;
        // don't skip sleeping here fully — we need to wake if pushed, but allow sleeping to be pushed out
        float radius;
        if (rb->type == object_sphere) radius = rb->radius;
        else {
            radius = rb->half_extensions.x;
            if (rb->half_extensions.y > radius) radius = rb->half_extensions.y;
            if (rb->half_extensions.z > radius) radius = rb->half_extensions.z;
            radius = sqrtf(radius*radius*3.0f) + 0.5f; // bound + voxel half
        }

        int bx_min = (int) floorf (rb->position.x - radius);
        int bx_max = (int) floorf (rb->position.x + radius);
        int by_min = (int) floorf (rb->position.y - radius);
        int by_max = (int) floorf (rb->position.y + radius);
        int bz_min = (int) floorf (rb->position.z - radius);
        int bz_max = (int) floorf (rb->position.z + radius);

        bool touched = false;
        vector3 accumulated_normal = {0,0,0};
        float max_pen = 0;

        for (int bx = bx_min; bx <= bx_max; bx++) {
            for (int by = by_min; by <= by_max; by++) {
                for (int bz = bz_min; bz <= bz_max; bz++) {
                    int bid = world_get (&pw_world, bx, by, bz);
                    const block_type *bt = block_type_get (bid);
                    if (!bt->solid) continue;

                    vector3 n; float pen;
                    if (!obb_voxel_collide(rb, bx,by,bz, &n, &pen)) continue;
                    if (pen <= 0.001f) continue;

                    // accumulate
                    rb->position.x += n.x * pen * 1.02f;
                    rb->position.y += n.y * pen * 1.02f;
                    rb->position.z += n.z * pen * 1.02f;

                    accumulated_normal.x += n.x;
                    accumulated_normal.y += n.y;
                    accumulated_normal.z += n.z;
                    if (pen > max_pen) max_pen = pen;

                    // velocity impulse: remove incoming normal velocity with restitution, apply friction
                    float vel_dot_n = rb->velocity.x*n.x + rb->velocity.y*n.y + rb->velocity.z*n.z;
                    if (vel_dot_n < 0) {
                        float e = fminf(rb->restitution, bt->restitution);
                        // if very low speed, no bounce
                        if (fabsf(vel_dot_n) < 0.5f) e = 0.0f;
                        float j = -(1.0f + e) * vel_dot_n;
                        rb->velocity.x += n.x * j;
                        rb->velocity.y += n.y * j;
                        rb->velocity.z += n.z * j;
                        // friction
                        float mu = fminf(rb->friction_kinetic, bt->friction_kinetic);
                        // simplified: reduce tangent after normal impulse
                        rb->velocity.x *= (1.0f - mu*0.1f);
                        rb->velocity.z *= (1.0f - mu*0.1f);
                        if (fabsf(rb->velocity.x) < 0.01f) rb->velocity.x = 0;
                        if (fabsf(rb->velocity.z) < 0.01f) rb->velocity.z = 0;
                    }

                    // angular damping on ground
                    if (n.y > 0.5f) {
                        rb->angular_velocity.x *= 0.9f;
                        rb->angular_velocity.z *= 0.9f;
                        if (fabsf(rb->velocity.y) < 0.2f) rb->velocity.y = 0;
                    }

                    touched = true;
                    // wake if was sleeping and meaningful penetration
                    if (rb->is_sleeping && pen > 0.02f) rigidbody_wake(rb);
                }
            }
        }

        // if multiple contacts, average normal for extra stability?
        (void) accumulated_normal; (void) max_pen;

        if (touched && rb->is_sleeping) rigidbody_wake(rb);
    }
}
