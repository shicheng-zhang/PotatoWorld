/* v10S — Block Rendering
 * GPU-instanced chunk meshing with own GLSL Phong shader. */

#ifndef game_block_render_h
#define game_block_render_h

#include "world.h"
#include <epoxy/gl.h>

void block_render_init (void);
void block_render_world (const game_world *w,
                         const float *projection_flat,
                         const float *view_flat,
                         float cam_x, float cam_y, float cam_z);
void block_render_world_sun (const game_world *w,
                             const float *projection_flat,
                             const float *view_flat,
                             float cam_x, float cam_y, float cam_z,
                             float sun_x, float sun_y, float sun_z);
void block_render_cleanup (void);

#endif
