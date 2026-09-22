/* PotatoWorld v2 — Block Rendering
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
void block_render_cleanup (void);

#endif
