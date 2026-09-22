/* PotatoWorld v2 — World Grid
 * 8x4x8 chunk grid (128x64x128 blocks), Perlin noise terrain. */

#ifndef game_world_h
#define game_world_h

#include "chunk.h"
#include <stdbool.h>

#define WORLD_CHUNKS_X 8
#define WORLD_CHUNKS_Y 4
#define WORLD_CHUNKS_Z 8

typedef struct {
    chunk chunks [WORLD_CHUNKS_X * WORLD_CHUNKS_Y * WORLD_CHUNKS_Z];
    int seed;
} game_world;

void world_init (game_world *w, int seed);
void world_destroy (game_world *w);
int world_get (const game_world *w, int wx, int wy, int wz);
void world_set (game_world *w, int wx, int wy, int wz, int block_id);
chunk *world_get_chunk (game_world *w, int cx, int cy, int cz);
void world_generate_flat (game_world *w);
void world_generate_terrain (game_world *w);

#endif
