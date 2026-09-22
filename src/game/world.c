/* PotatoWorld v2 — World Grid Implementation */

#include "world.h"
#include "block.h"
#include <string.h>
#include <stdlib.h>

static int world_chunk_index (int cx, int cy, int cz) {
    return (cy * WORLD_CHUNKS_Z + cz) * WORLD_CHUNKS_X + cx;
}

void world_init (game_world *w, int seed) {
    w->seed = seed;
    for (int cy = 0; cy < WORLD_CHUNKS_Y; cy++) {
        for (int cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            for (int cx = 0; cx < WORLD_CHUNKS_X; cx++) {
                chunk_init (&w->chunks [world_chunk_index (cx, cy, cz)], cx, cy, cz);
            }
        }
    }
}

void world_destroy (game_world *w) {
    for (int cy = 0; cy < WORLD_CHUNKS_Y; cy++) {
        for (int cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            for (int cx = 0; cx < WORLD_CHUNKS_X; cx++) {
                chunk_clear (&w->chunks [world_chunk_index (cx, cy, cz)]);
            }
        }
    }
}

int world_get (const game_world *w, int wx, int wy, int wz) {
    int cx = wx / CHUNK_SIZE;
    int cy = wy / CHUNK_SIZE;
    int cz = wz / CHUNK_SIZE;
    if (wx < 0) {cx = (wx + 1) / CHUNK_SIZE - 1;}
    if (wy < 0) {cy = (wy + 1) / CHUNK_SIZE - 1;}
    if (wz < 0) {cz = (wz + 1) / CHUNK_SIZE - 1;}
    if (cx < 0 || cx >= WORLD_CHUNKS_X) {return BLOCK_AIR;}
    if (cy < 0 || cy >= WORLD_CHUNKS_Y) {return BLOCK_AIR;}
    if (cz < 0 || cz >= WORLD_CHUNKS_Z) {return BLOCK_AIR;}
    int lx = wx - cx * CHUNK_SIZE;
    int ly = wy - cy * CHUNK_SIZE;
    int lz = wz - cz * CHUNK_SIZE;
    return chunk_get (&w->chunks [world_chunk_index (cx, cy, cz)], lx, ly, lz);
}

void world_set (game_world *w, int wx, int wy, int wz, int block_id) {
    int cx = wx / CHUNK_SIZE;
    int cy = wy / CHUNK_SIZE;
    int cz = wz / CHUNK_SIZE;
    if (wx < 0) {cx = (wx + 1) / CHUNK_SIZE - 1;}
    if (wy < 0) {cy = (wy + 1) / CHUNK_SIZE - 1;}
    if (wz < 0) {cz = (wz + 1) / CHUNK_SIZE - 1;}
    if (cx < 0 || cx >= WORLD_CHUNKS_X) {return;}
    if (cy < 0 || cy >= WORLD_CHUNKS_Y) {return;}
    if (cz < 0 || cz >= WORLD_CHUNKS_Z) {return;}
    int lx = wx - cx * CHUNK_SIZE;
    int ly = wy - cy * CHUNK_SIZE;
    int lz = wz - cz * CHUNK_SIZE;
    chunk *c = &w->chunks [world_chunk_index (cx, cy, cz)];
    chunk_set (c, lx, ly, lz, block_id);

    /* Mark neighboring chunks dirty if on a boundary */
    if (lx == 0 && cx > 0)                    {chunk_mark_dirty (&w->chunks [world_chunk_index (cx - 1, cy, cz)]);}
    if (lx == CHUNK_SIZE - 1 && cx < WORLD_CHUNKS_X - 1) {chunk_mark_dirty (&w->chunks [world_chunk_index (cx + 1, cy, cz)]);}
    if (ly == 0 && cy > 0)                    {chunk_mark_dirty (&w->chunks [world_chunk_index (cx, cy - 1, cz)]);}
    if (ly == CHUNK_SIZE - 1 && cy < WORLD_CHUNKS_Y - 1) {chunk_mark_dirty (&w->chunks [world_chunk_index (cx, cy + 1, cz)]);}
    if (lz == 0 && cz > 0)                    {chunk_mark_dirty (&w->chunks [world_chunk_index (cx, cy, cz - 1)]);}
    if (lz == CHUNK_SIZE - 1 && cz < WORLD_CHUNKS_Z - 1) {chunk_mark_dirty (&w->chunks [world_chunk_index (cx, cy, cz + 1)]);}
}

chunk *world_get_chunk (game_world *w, int cx, int cy, int cz) {
    if (cx < 0 || cx >= WORLD_CHUNKS_X) {return NULL;}
    if (cy < 0 || cy >= WORLD_CHUNKS_Y) {return NULL;}
    if (cz < 0 || cz >= WORLD_CHUNKS_Z) {return NULL;}
    return &w->chunks [world_chunk_index (cx, cy, cz)];
}

/* Simple hash for deterministic pseudo-random terrain */
static unsigned int terrain_hash (int x, int z, int seed) {
    unsigned int h = (unsigned int) seed;
    h ^= (unsigned int) x * 0x9e3779b9u;
    h ^= (unsigned int) z * 0x85ebca6bu;
    h = (h ^ (h >> 13)) * 0xc2b2ae35u;
    h = h ^ (h >> 16);
    return h;
}

void world_generate_flat (game_world *w) {
    for (int wz = 0; wz < WORLD_CHUNKS_Z * CHUNK_SIZE; wz++) {
        for (int wx = 0; wx < WORLD_CHUNKS_X * CHUNK_SIZE; wx++) {
            world_set (w, wx, 0, wz, BLOCK_GRASS);
            world_set (w, wx, -1, wz, BLOCK_DIRT);
            world_set (w, wx, -2, wz, BLOCK_STONE);
        }
    }
}

void world_generate_terrain (game_world *w) {
    int world_xz_size = WORLD_CHUNKS_X * CHUNK_SIZE;
    int world_z_size = WORLD_CHUNKS_Z * CHUNK_SIZE;

    for (int wz = 0; wz < world_z_size; wz++) {
        for (int wx = 0; wx < world_xz_size; wx++) {
            unsigned int h = terrain_hash (wx, wz, w->seed);
            int height = 2 + (int) (h % 6);

            for (int wy = -3; wy < height; wy++) {
                if (wy == height - 1) {
                    world_set (w, wx, wy, wz, BLOCK_GRASS);
                } else if (wy >= height - 3) {
                    world_set (w, wx, wy, wz, BLOCK_DIRT);
                } else {
                    world_set (w, wx, wy, wz, BLOCK_STONE);
                }
            }
        }
    }
}
