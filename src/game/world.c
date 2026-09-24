/* v10S — World Grid Implementation
 * Perlin noise terrain, ores, bedrock, trees. */

#include "world.h"
#include "block.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

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
    int old_block = chunk_get (c, lx, ly, lz);
    if (old_block == block_id) {return;}
    chunk_set (c, lx, ly, lz, block_id);

    const block_type *new_bt = block_type_get (block_id);
    const block_type *old_bt = block_type_get (old_block);

    if (old_bt && old_bt->on_neighbor_change) {
        int neighbor_offsets [6][3] = {{-1,0,0},{1,0,0},{0,-1,0},{0,1,0},{0,0,-1},{0,0,1}};
        for (int ni = 0; ni < 6; ni++) {
            int nx = wx + neighbor_offsets [ni][0];
            int ny = wy + neighbor_offsets [ni][1];
            int nz = wz + neighbor_offsets [ni][2];
            int ncx = nx / CHUNK_SIZE, ncy = ny / CHUNK_SIZE, ncz = nz / CHUNK_SIZE;
            if (nx < 0) {ncx = (nx + 1) / CHUNK_SIZE - 1;}
            if (ny < 0) {ncy = (ny + 1) / CHUNK_SIZE - 1;}
            if (nz < 0) {ncz = (nz + 1) / CHUNK_SIZE - 1;}
            if (ncx < 0 || ncx >= WORLD_CHUNKS_X || ncy < 0 || ncy >= WORLD_CHUNKS_Y || ncz < 0 || ncz >= WORLD_CHUNKS_Z) continue;
            int nlx = nx - ncx * CHUNK_SIZE;
            int nly = ny - ncy * CHUNK_SIZE;
            int nlz = nz - ncz * CHUNK_SIZE;
            int neighbor_id = chunk_get (&w->chunks [world_chunk_index (ncx, ncy, ncz)], nlx, nly, nlz);
            const block_type *neighbor_bt = block_type_get (neighbor_id);
            if (neighbor_bt && neighbor_bt->on_neighbor_change) {
                neighbor_bt->on_neighbor_change (w, nx, ny, nz, wx, wy, wz);
            }
        }
    }

    if (new_bt && new_bt->on_neighbor_change) {
        int neighbor_offsets [6][3] = {{-1,0,0},{1,0,0},{0,-1,0},{0,1,0},{0,0,-1},{0,0,1}};
        for (int ni = 0; ni < 6; ni++) {
            int nx = wx + neighbor_offsets [ni][0];
            int ny = wy + neighbor_offsets [ni][1];
            int nz = wz + neighbor_offsets [ni][2];
            int ncx = nx / CHUNK_SIZE, ncy = ny / CHUNK_SIZE, ncz = nz / CHUNK_SIZE;
            if (nx < 0) {ncx = (nx + 1) / CHUNK_SIZE - 1;}
            if (ny < 0) {ncy = (ny + 1) / CHUNK_SIZE - 1;}
            if (nz < 0) {ncz = (nz + 1) / CHUNK_SIZE - 1;}
            if (ncx < 0 || ncx >= WORLD_CHUNKS_X || ncy < 0 || ncy >= WORLD_CHUNKS_Y || ncz < 0 || ncz >= WORLD_CHUNKS_Z) continue;
            int nlx = nx - ncx * CHUNK_SIZE;
            int nly = ny - ncy * CHUNK_SIZE;
            int nlz = nz - ncz * CHUNK_SIZE;
            int neighbor_id = chunk_get (&w->chunks [world_chunk_index (ncx, ncy, ncz)], nlx, nly, nlz);
            const block_type *neighbor_bt = block_type_get (neighbor_id);
            if (neighbor_bt && neighbor_bt->on_neighbor_change) {
                neighbor_bt->on_neighbor_change (w, nx, ny, nz, wx, wy, wz);
            }
        }
    }

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

/* ---- Perlin noise ---- */

static int perm[512];

static void perlin_init (int seed) {
    int p[256];
    for (int i=0;i<256;i++) p[i]=i;
    unsigned int s = (unsigned int)seed * 1664525u + 1013904223u;
    for (int i=255;i>0;i--) {
        s = s * 1664525u + 1013904223u;
        int j = s % (i+1);
        int tmp=p[i]; p[i]=p[j]; p[j]=tmp;
    }
    for (int i=0;i<512;i++) perm[i]=p[i&255];
}

static inline float fade (float t) { return t*t*t*(t*(t*6.0f-15.0f)+10.0f); }
static inline float lerp (float a,float b,float t) { return a + t*(b-a); }
static inline float grad2 (int hash,float x,float y) {
    int h = hash & 7;
    float u = h<4 ? x : y;
    float v = h<4 ? y : x;
    return ((h&1)? -u : u) + ((h&2)? -v : v);
}
static inline float grad3 (int hash,float x,float y,float z) {
    int h = hash & 15;
    float u = h<8 ? x : y;
    float v = h<4 ? y : (h==12||h==14 ? x : z);
    return ((h&1)? -u : u) + ((h&2)? -v : v);
}

static float perlin2D (float x,float y) {
    int xi = (int)floorf(x) & 255;
    int yi = (int)floorf(y) & 255;
    float xf = x - floorf(x);
    float yf = y - floorf(y);
    float u = fade(xf);
    float v = fade(yf);
    int aa = perm[perm[xi]+yi];
    int ab = perm[perm[xi]+yi+1];
    int ba = perm[perm[xi+1]+yi];
    int bb = perm[perm[xi+1]+yi+1];
    float x1 = lerp(grad2(aa,xf,yf), grad2(ba,xf-1.0f,yf), u);
    float x2 = lerp(grad2(ab,xf,yf-1.0f), grad2(bb,xf-1.0f,yf-1.0f), u);
    return lerp(x1,x2,v);
}
static float perlin3D (float x,float y,float z) {
    int xi=(int)floorf(x)&255, yi=(int)floorf(y)&255, zi=(int)floorf(z)&255;
    float xf=x-floorf(x), yf=y-floorf(y), zf=z-floorf(z);
    float u=fade(xf), v=fade(yf), w=fade(zf);
    int aaa=perm[perm[perm[xi]+yi]+zi];
    int aba=perm[perm[perm[xi]+yi+1]+zi];
    int aab=perm[perm[perm[xi]+yi]+zi+1];
    int abb=perm[perm[perm[xi]+yi+1]+zi+1];
    int baa=perm[perm[perm[xi+1]+yi]+zi];
    int bba=perm[perm[perm[xi+1]+yi+1]+zi];
    int bab=perm[perm[perm[xi+1]+yi]+zi+1];
    int bbb=perm[perm[perm[xi+1]+yi+1]+zi+1];
    float x1=lerp(grad3(aaa,xf,yf,zf), grad3(baa,xf-1,yf,zf), u);
    float x2=lerp(grad3(aba,xf,yf-1,zf), grad3(bba,xf-1,yf-1,zf), u);
    float y1=lerp(x1,x2,v);
    x1=lerp(grad3(aab,xf,yf,zf-1), grad3(bab,xf-1,yf,zf-1), u);
    x2=lerp(grad3(abb,xf,yf-1,zf-1), grad3(bbb,xf-1,yf-1,zf-1), u);
    float y2=lerp(x1,x2,v);
    return lerp(y1,y2,w);
}

static float fractal_perlin (float x,float y) {
    float amp=1.0f, freq=0.02f, sum=0.0f, norm=0.0f;
    for (int o=0;o<4;o++) {
        sum += perlin2D(x*freq, y*freq) * amp;
        norm += amp;
        amp *= 0.5f;
        freq *= 2.0f;
    }
    return sum / norm; // -1..1
}

static unsigned int hash2 (int x,int z,int seed) {
    unsigned int h = (unsigned int)seed;
    h ^= (unsigned int)x * 0x9e3779b9u;
    h ^= (unsigned int)z * 0x85ebca6bu;
    h = (h ^ (h>>13)) * 0xc2b2ae35u;
    h = h ^ (h>>16);
    return h;
}

void world_generate_flat (game_world *w) {
    for (int wz = 0; wz < WORLD_CHUNKS_Z * CHUNK_SIZE; wz++) {
        for (int wx = 0; wx < WORLD_CHUNKS_X * CHUNK_SIZE; wx++) {
            world_set (w, wx, 0, wz, BLOCK_GRASS);
            world_set (w, wx, -1, wz, BLOCK_DIRT);
            world_set (w, wx, -2, wz, BLOCK_STONE);
            world_set (w, wx, -3, wz, BLOCK_STONE);
        }
    }
    /* bedrock floor */
    for (int wz = 0; wz < WORLD_CHUNKS_Z*CHUNK_SIZE; wz++)
        for (int wx = 0; wx < WORLD_CHUNKS_X*CHUNK_SIZE; wx++)
            world_set(w, wx, -4, wz, BLOCK_STONE);
}

static void generate_tree (game_world *w,int x,int y,int z) {
    int trunk_h = 4 + (hash2(x,z,w->seed+999) % 2);
    for (int i=0;i<trunk_h;i++) {
        if (y+i < WORLD_CHUNKS_Y*CHUNK_SIZE) world_set(w,x,y+i,z,BLOCK_OAK_LOG);
    }
    int top = y+trunk_h;
    for (int dx=-2;dx<=2;dx++) for (int dz=-2;dz<=2;dz++) for (int dy=-1;dy<=1;dy++) {
        if (dx==0 && dz==0 && dy<=0) continue;
        if (abs(dx)==2 && abs(dz)==2 && dy==1) continue;
        int lx=x+dx, ly=top+dy, lz=z+dz;
        if (world_get(w,lx,ly,lz)==BLOCK_AIR) world_set(w,lx,ly,lz,BLOCK_OAK_LEAVES);
    }
    if (world_get(w,x,top+1,z)==BLOCK_AIR) world_set(w,x,top+1,z,BLOCK_OAK_LEAVES);
    if (world_get(w,x,top+2,z)==BLOCK_AIR) world_set(w,x,top+2,z,BLOCK_OAK_LEAVES);
}

void world_generate_terrain (game_world *w) {
    perlin_init(w->seed);
    int world_xz_size = WORLD_CHUNKS_X * CHUNK_SIZE;
    int world_z_size = WORLD_CHUNKS_Z * CHUNK_SIZE;

    for (int wz = 0; wz < world_z_size; wz++) {
        for (int wx = 0; wx < world_xz_size; wx++) {
            float n = fractal_perlin((float)wx, (float)wz);
            float biome_n = perlin2D(wx*0.008f, wz*0.008f);
            int biome = 0; // 0 plains, 1 desert, 2 forest
            if (biome_n < -0.33f) biome = 1;
            else if (biome_n > 0.33f) biome = 2;
            int height;
            if (biome==1) height = 6 + (int)(n * 2.5f); // desert flat
            else if (biome==2) height = 9 + (int)(n * 7.0f); // forest hilly
            else height = 8 + (int)(n * 6.0f);
            if (height < 3) height = 3;
            if (height > 20) height = 20;
            if (height >= WORLD_CHUNKS_Y*CHUNK_SIZE-5) height = WORLD_CHUNKS_Y*CHUNK_SIZE-6;

            for (int wy = -3; wy < height; wy++) {
                int existing = world_get(w, wx, wy, wz);
                if (existing != BLOCK_AIR) continue;
                unsigned int h = hash2(wx*3+wy*7, wz*5+wy*11, w->seed + wy*100);
                int block = BLOCK_STONE;
                if (biome==1) { // desert
                    if (wy == height - 1) block = BLOCK_SAND;
                    else if (wy >= height - 4) block = BLOCK_SAND;
                    else block = BLOCK_STONE;
                } else {
                    if (wy == height - 1) {
                        block = BLOCK_GRASS;
                        if (height < 6 && (h % 4)==0) block = BLOCK_SAND;
                    } else if (wy >= height - 3) {
                        block = BLOCK_DIRT;
                        if (height < 6 && (h % 5)==0) block = BLOCK_SAND;
                    } else {
                        int r = h % 1000;
                        if (wy < 2 && r < 5) block = BLOCK_DIAMOND_ORE;
                        else if (wy == 1 && r < 8) block = BLOCK_BEDROCK;
                        else if (wy < 4 && r < 12) block = BLOCK_GOLD_ORE;
                        else if (wy < 8 && r < 30) block = BLOCK_IRON_ORE;
                        else if (wy < 10 && r < 25) block = BLOCK_COAL_ORE;
                        else if (wy < 6 && r < 40) block = BLOCK_GRAVEL;
                        else if (wy < height-4 && r < 40) block = (r < 20) ? BLOCK_DIRT : BLOCK_STONE;
                    }
                }
                // caves: carve with 3D noise, avoid surface
                if (wy > 3 && wy < height-2) {
                    float cave = perlin3D(wx*0.06f, wy*0.06f, wz*0.06f);
                    if (cave > 0.62f) continue; // leave air (no block)
                    // worm-like secondary
                    float cave2 = perlin3D(wx*0.12f+100, wy*0.12f, wz*0.12f+100);
                    if (cave2 > 0.58f && (h%7)==0) continue;
                }
                world_set (w, wx, wy, wz, block);
            }
            // water lakes for low terrain
            if (height < 5) {
                for(int wy=height; wy<=4; wy++){
                    if(world_get(w,wx,wy,wz)==BLOCK_AIR) world_set(w,wx,wy,wz,BLOCK_WATER);
                }
            }
            // trees / cactus
            if (biome==1) {
                // desert cactus 1%
                unsigned int th = hash2(wx, wz, w->seed+54321);
                if ((th % 200) < 2) {
                    int top_y = height -1;
                    if(world_get(w,wx,top_y,wz)==BLOCK_SAND && world_get(w,wx,top_y+1,wz)==BLOCK_AIR){
                        int h = 2 + (hash2(wx,wz,w->seed+999)%2);
                        for(int i=0;i<h;i++) if(world_get(w,wx,top_y+1+i,wz)==BLOCK_AIR) world_set(w,wx,top_y+1+i,wz,BLOCK_OAK_LOG); // reuse log as cactus
                    }
                }
            } else if (biome!=1 && height >= 6 && height < 18) {
                float forest_density = (biome==2)?6.0f:3.0f;
                unsigned int th = hash2(wx, wz, w->seed+12345);
                if ((int)(th % 100) < (int)forest_density) {
                    int top_y = height - 1;
                    if (world_get(w, wx, top_y, wz)==BLOCK_GRASS && world_get(w, wx, top_y+1, wz)==BLOCK_AIR)
                        generate_tree(w, wx, top_y+1, wz);
                }
            }
        }
    }
    // bedrock at y=0
    for(int wz=0;wz<world_z_size;wz++) for(int wx=0;wx<world_xz_size;wx++) world_set(w,wx,0,wz,BLOCK_BEDROCK);
    // ensure spawn area flat 3x3 for player
    for(int dx=-1;dx<=1;dx++) for(int dz=-1;dz<=1;dz++){
        int x=64+dx, z=64+dz;
        for(int y=1;y<6;y++) world_set(w,x,y,z,BLOCK_AIR);
        world_set(w,x,0,z,BLOCK_BEDROCK);
        world_set(w,x,1,z,BLOCK_GRASS);
    }
}

void world_tick_falling (game_world *w) {
    /* simple falling sand: scan from bottom up, let sand drop if air below */
    for (int y = 1; y < WORLD_CHUNKS_Y*CHUNK_SIZE; y++) {
        for (int z = 0; z < WORLD_CHUNKS_Z*CHUNK_SIZE; z++) {
            for (int x = 0; x < WORLD_CHUNKS_X*CHUNK_SIZE; x++) {
                int bid = world_get(w, x, y, z);
                if (bid != BLOCK_SAND) continue;
                const block_type *bt = block_type_get(bid);
                if (!(bt->tags & TAG_FALLING)) continue;
                int below = world_get(w, x, y-1, z);
                if (below == BLOCK_AIR) {
                    world_set(w, x, y, z, BLOCK_AIR);
                    world_set(w, x, y-1, z, BLOCK_SAND);
                }
            }
        }
    }
}
