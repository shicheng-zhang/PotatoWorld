/* v10S — Chunk System Implementation */

#include "chunk.h"
#include <string.h>

void chunk_init (chunk *c, int cx, int cy, int cz) {
    c->cx = cx;
    c->cy = cy;
    c->cz = cz;
    memset (c->blocks, 0, sizeof (c->blocks));
    c->dirty = true;
    c->vao = 0;
    c->vbo = 0;
    c->ebo = 0;
    c->index_count = 0;
}

void chunk_clear (chunk *c) {
    if (c->vao) {glDeleteVertexArrays (1, &c->vao); c->vao = 0;}
    if (c->vbo) {glDeleteBuffers (1, &c->vbo); c->vbo = 0;}
    if (c->ebo) {glDeleteBuffers (1, &c->ebo); c->ebo = 0;}
    c->index_count = 0;
    c->dirty = true;
}

static int chunk_index (int lx, int ly, int lz) {
    return (ly * CHUNK_SIZE + lz) * CHUNK_SIZE + lx;
}

int chunk_get (const chunk *c, int lx, int ly, int lz) {
    if (lx < 0 || lx >= CHUNK_SIZE) {return 0;}
    if (ly < 0 || ly >= CHUNK_SIZE) {return 0;}
    if (lz < 0 || lz >= CHUNK_SIZE) {return 0;}
    return c->blocks [chunk_index (lx, ly, lz)];
}

void chunk_set (chunk *c, int lx, int ly, int lz, int block_id) {
    if (lx < 0 || lx >= CHUNK_SIZE) {return;}
    if (ly < 0 || ly >= CHUNK_SIZE) {return;}
    if (lz < 0 || lz >= CHUNK_SIZE) {return;}
    c->blocks [chunk_index (lx, ly, lz)] = (unsigned char) block_id;
    c->dirty = true;
}

int chunk_needs_rebuild (const chunk *c) {
    return c->dirty;
}

void chunk_mark_dirty (chunk *c) {
    c->dirty = true;
}
