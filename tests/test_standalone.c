/* Standalone headless stability test — no GL, no engine linking.
 * Tests world collision math + chunk integrity directly. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#define CHUNK_SIZE 16
#define WORLD_CHUNKS_X 8
#define WORLD_CHUNKS_Y 4
#define WORLD_CHUNKS_Z 8
#define BLOCK_AIR 0

typedef struct { float x, y, z; } vec3;

typedef struct {
    uint8_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    int dirty;
} chunk;

typedef struct {
    chunk chunks[WORLD_CHUNKS_X][WORLD_CHUNKS_Y][WORLD_CHUNKS_Z];
} game_world;

static game_world pw_world;

static inline int world_to_chunk_local(int wx, int wy, int wz, int *cx, int *cy, int *cz, int *lx, int *ly, int *lz) {
    *cx = wx / CHUNK_SIZE; *cy = wy / CHUNK_SIZE; *cz = wz / CHUNK_SIZE;
    *lx = wx - (*cx) * CHUNK_SIZE; *ly = wy - (*cy) * CHUNK_SIZE; *lz = wz - (*cz) * CHUNK_SIZE;
    if (*lx < 0) { *lx += CHUNK_SIZE; (*cx)--; }
    if (*ly < 0) { *ly += CHUNK_SIZE; (*cy)--; }
    if (*lz < 0) { *lz += CHUNK_SIZE; (*cz)--; }
    return (*cx >= 0 && *cx < WORLD_CHUNKS_X && *cy >= 0 && *cy < WORLD_CHUNKS_Y && *cz >= 0 && *cz < WORLD_CHUNKS_Z);
}

static inline uint8_t world_get_block(int x, int y, int z) {
    int cx, cy, cz, lx, ly, lz;
    if (!world_to_chunk_local(x, y, z, &cx, &cy, &cz, &lx, &ly, &lz)) return BLOCK_AIR;
    return pw_world.chunks[cx][cy][cz].blocks[lx][ly][lz];
}

static inline void world_set_block(int x, int y, int z, uint8_t type) {
    int cx, cy, cz, lx, ly, lz;
    if (!world_to_chunk_local(x, y, z, &cx, &cy, &cz, &lx, &ly, &lz)) return;
    pw_world.chunks[cx][cy][cz].blocks[lx][ly][lz] = type;
    pw_world.chunks[cx][cy][cz].dirty = 1;
}

static void generate_flat_terrain(int height) {
    memset(&pw_world, 0, sizeof(pw_world));
    int total_x = WORLD_CHUNKS_X * CHUNK_SIZE;
    int total_z = WORLD_CHUNKS_Z * CHUNK_SIZE;
    for (int x = 0; x < total_x; x++) {
        for (int z = 0; z < total_z; z++) {
            world_set_block(x, 0, z, 1);
            for (int y = 1; y < height && y < WORLD_CHUNKS_Y * CHUNK_SIZE; y++) {
                world_set_block(x, y, z, y == height - 1 ? 2 : 3);
            }
        }
    }
}

/* Simple sphere-vs-voxel AABB collision */
typedef struct {
    vec3 position;
    vec3 velocity;
    float radius;
    float mass;
    int sleeping;
    int static_state;
    int fallen;
} test_body;

static int test_body_collide_voxel(test_body *b, int world_x, int world_y, int world_z) {
    float vx = (float)world_x + 0.5f;
    float vy = (float)world_y + 0.5f;
    float vz = (float)world_z + 0.5f;
    float dx = b->position.x - vx;
    float dy = b->position.y - vy;
    float dz = b->position.z - vz;
    float half = 0.5f;
    float closest_x = fmaxf(-half, fminf(dx, half));
    float closest_y = fmaxf(-half, fminf(dy, half));
    float closest_z = fmaxf(-half, fminf(dz, half));
    float dist_x = dx - closest_x;
    float dist_y = dy - closest_y;
    float dist_z = dz - closest_z;
    float dist_sq = dist_x * dist_x + dist_y * dist_y + dist_z * dist_z;
    if (dist_sq >= b->radius * b->radius) return 0;
    float dist = sqrtf(dist_sq);
    if (dist < 1e-6f) {
        b->position.y += b->radius + 0.01f;
        b->velocity.y = 0;
        return 1;
    }
    float nx = dist_x / dist, ny = dist_y / dist, nz = dist_z / dist;
    float penetration = b->radius - dist;
    b->position.x += nx * penetration;
    b->position.y += ny * penetration;
    b->position.z += nz * penetration;
    float vn = b->velocity.x * nx + b->velocity.y * ny + b->velocity.z * nz;
    if (vn < 0) {
        b->velocity.x -= (1.0f + 0.3f) * vn * nx;
        b->velocity.y -= (1.0f + 0.3f) * vn * ny;
        b->velocity.z -= (1.0f + 0.3f) * vn * nz;
        float friction_tangent = 0.5f;
        float vt = b->velocity.x * (-ny) + b->velocity.z * (-nx);
        if (fabsf(vt) > 0.01f) {
            float factor = fminf(friction_tangent * fabsf(vn), fabsf(vt));
            float sign = vt > 0 ? 1.0f : -1.0f;
            b->velocity.x += -ny * factor * sign;
            b->velocity.z += -nx * factor * sign;
        }
    }
    return 1;
}

static void body_voxel_collide(test_body *b) {
    int min_x = (int)floorf(b->position.x - b->radius);
    int max_x = (int)ceilf(b->position.x + b->radius);
    int min_y = (int)floorf(b->position.y - b->radius);
    int max_y = (int)ceilf(b->position.y + b->radius);
    int min_z = (int)floorf(b->position.z - b->radius);
    int max_z = (int)ceilf(b->position.z + b->radius);
    for (int x = min_x; x <= max_x; x++)
        for (int y = min_y; y <= max_y; y++)
            for (int z = min_z; z <= max_z; z++)
                if (world_get_block(x, y, z) != BLOCK_AIR)
                    test_body_collide_voxel(b, x, y, z);
}

int main(void) {
    printf("=== v10S Headless Stability Test ===\n");

    /* 1. Terrain generation test */
    generate_flat_terrain(4);
    printf("[OK] Flat terrain generated (height 4)\n");

    /* Verify solid blocks at expected positions */
    int solid_count = 0;
    for (int x = 0; x < 128; x++)
        for (int z = 0; z < 128; z++)
            for (int y = 0; y < 4; y++)
                if (world_get_block(x, y, z) != BLOCK_AIR) solid_count++;
    int expected = 128 * 128 * 4;
    printf("[CHECK] Solid blocks: %d / %d (expected %d)\n", solid_count, expected, expected);
    if (solid_count != expected) { printf("[FAIL]\n"); return 1; }
    printf("[OK] Terrain integrity: PASS\n");

    /* 2. Spawn 50 spheres above terrain and simulate */
    const int NUM_SPHERES = 50;
    test_body bodies[50];
    memset(bodies, 0, sizeof(bodies));
    for (int i = 0; i < NUM_SPHERES; i++) {
        bodies[i].position = (vec3){-5.0f + (i % 10) * 1.2f, 10.0f + (i / 10) * 2.0f, -5.0f + (i / 10) * 1.2f};
        bodies[i].velocity = (vec3){0, 0, 0};
        bodies[i].radius = 0.35f;
        bodies[i].mass = 1.0f;
    }
    printf("[OK] Spawned %d test spheres\n", NUM_SPHERES);

    /* 3. Run 600 ticks (10 seconds at 60Hz) */
    const int total_ticks = 600;
    const float dt = 1.0f / 60.0f;
    const float gravity = -9.81f;
    int nan_count = 0;
    int fallen = 0;
    float max_speed = 0.0f;

    printf("[RUN] Simulating %d ticks...\n", total_ticks);

    for (int tick = 0; tick < total_ticks; tick++) {
        for (int i = 0; i < NUM_SPHERES; i++) {
            test_body *b = &bodies[i];
            if (b->sleeping || b->static_state) continue;

            /* Gravity */
            b->velocity.y += gravity * dt;

            /* Damping */
            b->velocity.x *= 0.99f;
            b->velocity.y *= 0.99f;
            b->velocity.z *= 0.99f;

            /* Integrate position */
            b->position.x += b->velocity.x * dt;
            b->position.y += b->velocity.y * dt;
            b->position.z += b->velocity.z * dt;

            /* Floor */
            if (b->position.y < b->radius) {
                b->position.y = b->radius;
                b->velocity.y *= -0.3f;
                if (fabsf(b->velocity.y) < 0.01f) b->velocity.y = 0;
                float friction = 0.5f;
                float speed = sqrtf(b->velocity.x * b->velocity.x + b->velocity.z * b->velocity.z);
                if (speed > 0.001f) {
                    float reduce = friction * fabsf(gravity) * dt;
                    if (reduce > speed) reduce = speed;
                    b->velocity.x -= (b->velocity.x / speed) * reduce;
                    b->velocity.z -= (b->velocity.z / speed) * reduce;
                }
            }

            /* Voxel collision */
            body_voxel_collide(b);

            /* NaN check */
            if (!isfinite(b->position.x) || !isfinite(b->position.y) || !isfinite(b->position.z)) nan_count++;
            if (!isfinite(b->velocity.x) || !isfinite(b->velocity.y) || !isfinite(b->velocity.z)) nan_count++;
            if (b->position.y < -10.0f) fallen++;

            float spd = sqrtf(b->velocity.x*b->velocity.x + b->velocity.y*b->velocity.y + b->velocity.z*b->velocity.z);
            if (spd > max_speed) max_speed = spd;
        }

        if (tick % 100 == 0) printf("  tick %d: max_speed=%.4f\n", tick, max_speed);
    }

    /* 4. Final checks */
    int sleeping = 0, awake = 0;
    for (int i = 0; i < NUM_SPHERES; i++) {
        float spd = sqrtf(bodies[i].velocity.x*bodies[i].velocity.x + bodies[i].velocity.y*bodies[i].velocity.y + bodies[i].velocity.z*bodies[i].velocity.z);
        if (spd < 0.01f) sleeping++;
        else awake++;
    }

    printf("\n=== Results ===\n");
    printf("Bodies: %d\n", NUM_SPHERES);
    printf("Sleeping: %d, Still moving: %d\n", sleeping, awake);
    printf("Max speed: %.6f\n", max_speed);
    printf("NaN events: %d\n", nan_count);
    printf("Fallen below -10: %d\n", fallen);

    bool pass = (nan_count == 0) && (fallen == 0) && (NUM_SPHERES > 0);
    printf("Overall: %s\n", pass ? "PASS" : "FAIL");

    return pass ? 0 : 1;
}
