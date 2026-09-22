/* PotatoWorld v2 — DDA Voxel Raycast Implementation */

#include "raycast.h"
#include "world.h"
#include "block.h"
#include <math.h>
#include <float.h>

raycast_result raycast_voxel (const game_world *w,
                              float origin_x, float origin_y, float origin_z,
                              float dir_x, float dir_y, float dir_z,
                              float max_distance) {
    raycast_result result = {0};
    result.hit = false;
    result.distance = max_distance;

    float length = sqrtf (dir_x * dir_x + dir_y * dir_y + dir_z * dir_z);
    if (length < 0.0001f) {return result;}
    dir_x /= length;
    dir_y /= length;
    dir_z /= length;

    int x = (int) floorf (origin_x);
    int y = (int) floorf (origin_y);
    int z = (int) floorf (origin_z);

    int step_x = (dir_x > 0) ? 1 : -1;
    int step_y = (dir_y > 0) ? 1 : -1;
    int step_z = (dir_z > 0) ? 1 : -1;

    float t_delta_x = (dir_x != 0.0f) ? fabsf (1.0f / dir_x) : FLT_MAX;
    float t_delta_y = (dir_y != 0.0f) ? fabsf (1.0f / dir_y) : FLT_MAX;
    float t_delta_z = (dir_z != 0.0f) ? fabsf (1.0f / dir_z) : FLT_MAX;

    float t_max_x = (dir_x > 0) ? ((float) (x + 1) - origin_x) / dir_x
                  : (dir_x < 0) ? (origin_x - (float) x) / -dir_x : FLT_MAX;
    float t_max_y = (dir_y > 0) ? ((float) (y + 1) - origin_y) / dir_y
                  : (dir_y < 0) ? (origin_y - (float) y) / -dir_y : FLT_MAX;
    float t_max_z = (dir_z > 0) ? ((float) (z + 1) - origin_z) / dir_z
                  : (dir_z < 0) ? (origin_z - (float) z) / -dir_z : FLT_MAX;

    int normal_x = 0, normal_y = 0, normal_z = 0;

    for (int i = 0; i < 256; i++) {
        int block_id = world_get (w, x, y, z);
        const block_type *bt = block_type_get (block_id);
        if (bt->solid) {
            result.block_x = x;
            result.block_y = y;
            result.block_z = z;
            result.normal_x = normal_x;
            result.normal_y = normal_y;
            result.normal_z = normal_z;
            result.distance = (t_max_x < t_max_y) ?
                              ((t_max_x < t_max_z) ? t_max_x : t_max_z) :
                              ((t_max_y < t_max_z) ? t_max_y : t_max_z);
            if (result.distance > max_distance) {result.hit = false; return result;}
            result.hit = true;
            return result;
        }

        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                if (t_max_x > max_distance) {break;}
                x += step_x;
                t_max_x += t_delta_x;
                normal_x = -step_x;
                normal_y = 0;
                normal_z = 0;
            } else {
                if (t_max_z > max_distance) {break;}
                z += step_z;
                t_max_z += t_delta_z;
                normal_x = 0;
                normal_y = 0;
                normal_z = -step_z;
            }
        } else {
            if (t_max_y < t_max_z) {
                if (t_max_y > max_distance) {break;}
                y += step_y;
                t_max_y += t_delta_y;
                normal_x = 0;
                normal_y = -step_y;
                normal_z = 0;
            } else {
                if (t_max_z > max_distance) {break;}
                z += step_z;
                t_max_z += t_delta_z;
                normal_x = 0;
                normal_y = 0;
                normal_z = -step_z;
            }
        }
    }

    return result;
}
