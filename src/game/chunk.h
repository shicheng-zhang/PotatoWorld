/* PotatoWorld v2 — Chunk System
 * 16x16x16 chunks with dirty-flag rebuild and GPU mesh handles. */

#ifndef game_chunk_h
#define game_chunk_h

#include <stdbool.h>
#include <epoxy/gl.h>

#define CHUNK_SIZE 16

typedef struct {
    int cx, cy, cz;
    unsigned char blocks [CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    bool dirty;
    GLuint vao, vbo, ebo;
    int index_count;
} chunk;

void chunk_init (chunk *c, int cx, int cy, int cz);
void chunk_clear (chunk *c);
int chunk_get (const chunk *c, int lx, int ly, int lz);
void chunk_set (chunk *c, int lx, int ly, int lz, int block_id);
int chunk_needs_rebuild (const chunk *c);
void chunk_mark_dirty (chunk *c);

#endif
