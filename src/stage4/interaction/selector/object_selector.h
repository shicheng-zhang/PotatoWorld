#ifndef object_selector_h
#define object_selector_h
#include "../../../stage1/master_header.h"
#include "../../../stage2/master_header_2.h"
//Index of current object in the counter
extern int selected_object;
//Ray trace from camera along the front/camera view vector
//This will hit the closest object in path
//-1 value returned if no object is hit
int selector_ray_tracing (void);
//Deselect object
void clear_selection (void);
//Get the position on the floor (y=0) where the camera is looking
bool selector_get_floor_hit (vector3 *hit_position);
//Get snapped position if near another cube
bool selector_get_snapped_hit (vector3 *snapped_position, float snap_buffer_distance, float new_cube_half_extent);
//Apply impulse to selected object in the view phase
void selector_apply_force_impulse (float impulse_magnitude);
#endif
