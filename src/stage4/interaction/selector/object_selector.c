#include "object_selector.h"
int selected_object = -1;
static int right_click_miss_count = 0;
int selector_ray_tracing (void) {
    vector3 ray_origin_position = main_camera_fov.position;
    vector3 ray_direction_vector = vector3_normalisation (main_camera_fov.forward_vector);
    float closest_hit_distance = 1e30;
    int closest_object_index = -1;
    for (int object_index = 0; object_index < object_count; object_index++) {
        rigidbody *rigid_body_pointer = &obj_per_scene [object_index];
        //Vector: Ray Origin to Sphere Centre
        vector3 origin_to_center_vector = vector3_subtraction (rigid_body_pointer -> position, ray_origin_position);
        //Project origin_centre onto ray direction
        float projection_length_along_ray = vector3_dot (origin_to_center_vector, ray_direction_vector);
        if (projection_length_along_ray < 0) {continue;} //Behind Camera
        //Perpendicular distance squared
        vector3 closest_point_on_ray_vector = vector3_scaling (ray_direction_vector, projection_length_along_ray);
        vector3 perpendicular_displacement_vector = vector3_subtraction (origin_to_center_vector, closest_point_on_ray_vector);
        float perpendicular_distance_squared = vector3_length_squared (perpendicular_displacement_vector);
        //Collision Check (Radius)
        if (perpendicular_distance_squared <= (rigid_body_pointer -> radius * rigid_body_pointer -> radius)) {
            if (projection_length_along_ray < closest_hit_distance) {
                closest_hit_distance = projection_length_along_ray;
                closest_object_index = object_index;
            }
        }
    } if (closest_object_index >= 0) {
        selected_object = closest_object_index;
        right_click_miss_count = 0;
    } else {
        right_click_miss_count++;
        if (right_click_miss_count >= 2) {
            selected_object = -1;
            right_click_miss_count = 0;
        }
    } return selected_object;
} void clear_selection (void) {
    selected_object = -1;
} bool selector_get_floor_hit (vector3 *hit_position) {
    vector3 ray_origin = main_camera_fov.position;
    vector3 ray_direction = vector3_normalisation (main_camera_fov.forward_vector);
    // Intersection with plane y = 0
    // ray_origin.y + t * ray_direction.y = 0  => t = -ray_origin.y / ray_direction.y
    if (fabsf (ray_direction.y) < 0.0001f) {return false;} // Parallel to floor
    float t = -ray_origin.y / ray_direction.y;
    if (t < 0) {return false;} // Pointing away from floor
    *hit_position = vector3_addition (ray_origin, vector3_scaling (ray_direction, t));
    return true;
} bool selector_get_snapped_hit (vector3 *snapped_position, float snap_buffer_distance, float new_cube_half_extent) {
    (void) snap_buffer_distance;
    vector3 ray_origin = main_camera_fov.position;
    vector3 ray_direction = vector3_normalisation (main_camera_fov.forward_vector);
    float closest_t = 1e30f;
    vector3 target_pos = {0, 0, 0};
    bool hit_found = false;
    for (int i = 0; i < object_count; i++) {
        rigidbody *rb = &obj_per_scene [i];
        if (rb -> type != object_cube) continue;
        vector3 box_min = vector3_subtraction (rb -> position, rb -> half_extensions);
        vector3 box_max = vector3_addition (rb -> position, rb -> half_extensions);
        float tmin = -1e30f, tmax = 1e30f;
        int near_axis = -1;
        float near_sign = 0;
        for (int axis = 0; axis < 3; axis++) {
            float origin = (axis == 0) ? ray_origin.x : (axis == 1) ? ray_origin.y : ray_origin.z;
            float dir = (axis == 0) ? ray_direction.x : (axis == 1) ? ray_direction.y : ray_direction.z;
            float min = (axis == 0) ? box_min.x : (axis == 1) ? box_min.y : box_min.z;
            float max = (axis == 0) ? box_max.x : (axis == 1) ? box_max.y : box_max.z;
            if (fabsf (dir) < 1e-6f) {
                if (origin < min || origin > max) { tmin = 1e31f; break; }
            } else {
                float invD = 1.0f / dir;
                float t1 = (min - origin) * invD;
                float t2 = (max - origin) * invD;
                float s1 = -1.0f, s2 = 1.0f;
                if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; s1 = 1.0f; s2 = -1.0f; }
                if (t1 > tmin) { tmin = t1; near_axis = axis; near_sign = s1; }
                if (t2 < tmax) tmax = t2;
            }
        } if (tmin <= tmax && tmax > 0) {
            float t = (tmin < 0) ? tmax : tmin;
            if (t < closest_t) {
                closest_t = t;
                hit_found = true;
                vector3 normal = {0, 0, 0};
                if (near_axis == 0) normal.x = near_sign;
                else if (near_axis == 1) normal.y = near_sign;
                else if (near_axis == 2) normal.z = near_sign;
                target_pos = vector3_addition (rb -> position, vector3_scaling (normal, new_cube_half_extent * 2.0f));
            }
        }
    } if (!hit_found) {
        if (fabsf (ray_direction.y) > 0.0001f) {
            float t = -ray_origin.y / ray_direction.y;
            if (t > 0 && t < closest_t) {
                vector3 floor_hit = vector3_addition (ray_origin, vector3_scaling (ray_direction, t));
                target_pos = floor_hit;
                target_pos.y = new_cube_half_extent;
                hit_found = true;
            }
        }
    } if (!hit_found) return false;
    float S = new_cube_half_extent * 2.0f;
    snapped_position -> x = floorf (target_pos.x / S) * S + new_cube_half_extent;
    snapped_position -> z = floorf (target_pos.z / S) * S + new_cube_half_extent;
    snapped_position -> y = floorf ((target_pos.y - new_cube_half_extent + 0.05f) / S) * S + new_cube_half_extent;
    if (snapped_position -> x < -250.0f + new_cube_half_extent) snapped_position -> x = -250.0f + new_cube_half_extent;
    if (snapped_position -> x > 250.0f - new_cube_half_extent) snapped_position -> x = 250.0f - new_cube_half_extent;
    if (snapped_position -> z < -250.0f + new_cube_half_extent) snapped_position -> z = -250.0f + new_cube_half_extent;
    if (snapped_position -> z > 250.0f - new_cube_half_extent) snapped_position -> z = 250.0f - new_cube_half_extent;
    if (snapped_position -> y < new_cube_half_extent) snapped_position -> y = new_cube_half_extent;
    return true;
} void selector_apply_force_impulse (float impulse_magnitude) {
    if ((selected_object < 0) || (selected_object >= object_count)) {return;}
    rigidbody *selected_rigid_body = &obj_per_scene [selected_object];
    vector3 applied_impulse_vector = vector3_scaling (main_camera_fov.forward_vector, impulse_magnitude);
    rb_apply_forces_perfect (selected_rigid_body, applied_impulse_vector);
}
