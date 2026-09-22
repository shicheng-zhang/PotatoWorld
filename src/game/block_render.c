/* PotatoWorld v2 — Block Rendering Implementation
 * GPU-instanced chunk meshing, own GLSL Phong shader. */

#include "block_render.h"
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "../core/math4_special.h"
#include <epoxy/gl.h>
#include <stdlib.h>
#include <string.h>

static GLuint block_shader = 0;
static GLint block_proj_loc = 0;
static GLint block_view_loc = 0;
static GLint block_cam_loc = 0;
static GLint block_light_loc = 0;

static const char *block_vert_src =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec3 aColor;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 viewframe;\n"
    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "out vec3 Color;\n"
    "void main() {\n"
    "    FragPos = aPos;\n"
    "    Normal = aNormal;\n"
    "    Color = aColor;\n"
    "    gl_Position = projection * viewframe * vec4(aPos, 1.0);\n"
    "}\n";

static const char *block_frag_src =
    "#version 330 core\n"
    "in vec3 FragPos;\n"
    "in vec3 Normal;\n"
    "in vec3 Color;\n"
    "uniform vec3 camera_position;\n"
    "uniform vec3 light_position;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    vec3 ambient = 0.5 * Color;\n"
    "    vec3 lightDir = normalize(light_position - FragPos);\n"
    "    float diff = max(dot(Normal, lightDir), 0.0);\n"
    "    vec3 diffuse = diff * 0.7 * Color;\n"
    "    vec3 viewDir = normalize(camera_position - FragPos);\n"
    "    vec3 reflectDir = reflect(-lightDir, Normal);\n"
    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);\n"
    "    vec3 specular = 0.15 * spec * vec3(1.0);\n"
    "    FragColor = vec4(ambient + diffuse + specular, 1.0);\n"
    "}\n";

static GLuint compile_shader (GLenum type, const char *src) {
    GLuint s = glCreateShader (type);
    glShaderSource (s, 1, &src, NULL);
    glCompileShader (s);
    GLint ok = 0;
    glGetShaderiv (s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log [512];
        glGetShaderInfoLog (s, sizeof (log), NULL, log);
        fprintf (stderr, "Block shader compile error: %s\n", log);
        glDeleteShader (s);
        return 0;
    }
    return s;
}

static GLuint create_block_shader (void) {
    GLuint vs = compile_shader (GL_VERTEX_SHADER, block_vert_src);
    GLuint fs = compile_shader (GL_FRAGMENT_SHADER, block_frag_src);
    if (!vs || !fs) {return 0;}
    GLuint prog = glCreateProgram ();
    glAttachShader (prog, vs);
    glAttachShader (prog, fs);
    glLinkProgram (prog);
    GLint ok = 0;
    glGetProgramiv (prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log [512];
        glGetProgramInfoLog (prog, sizeof (log), NULL, log);
        fprintf (stderr, "Block shader link error: %s\n", log);
        glDeleteProgram (prog);
        prog = 0;
    }
    glDeleteShader (vs);
    glDeleteShader (fs);
    return prog;
}

void block_render_init (void) {
    block_shader = create_block_shader ();
    if (!block_shader) {
        fprintf (stderr, "Block render init FAILED\n");
        return;
    }
    block_proj_loc = glGetUniformLocation (block_shader, "projection");
    block_view_loc = glGetUniformLocation (block_shader, "viewframe");
    block_cam_loc = glGetUniformLocation (block_shader, "camera_position");
    block_light_loc = glGetUniformLocation (block_shader, "light_position");
}

/* 6 faces: +x, -x, +y, -y, +z, -z */
/* Each face: 4 vertices (pos + normal + color), 2 triangles (6 indices) */

static const float face_norms [6][3] = {
    { 1, 0, 0}, {-1, 0, 0},
    { 0, 1, 0}, { 0,-1, 0},
    { 0, 0, 1}, { 0, 0,-1},
};
static const unsigned int face_indices [6][6] = {
    {0,1,2, 0,2,3},
    {0,1,2, 0,2,3},
    {0,1,2, 0,2,3},
    {0,1,2, 0,2,3},
    {0,1,2, 0,2,3},
    {0,1,2, 0,2,3},
};

/* Correct face vertex positions */
static const float face_verts_correct [6][4][3] = {
    /* +x */ {{1,0,0}, {1,1,0}, {1,1,1}, {1,0,1}},
    /* -x */ {{0,0,1}, {0,1,1}, {0,1,0}, {0,0,0}},
    /* +y */ {{0,1,0}, {0,1,1}, {1,1,1}, {1,1,0}},
    /* -y */ {{0,0,1}, {0,0,0}, {1,0,0}, {1,0,1}},
    /* +z */ {{0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}},
    /* -z */ {{1,0,0}, {0,0,0}, {0,1,0}, {1,1,0}},
};

static void build_chunk_mesh (const game_world *w, const chunk *c,
                              float **verts_out, unsigned int **inds_out,
                              int *vert_count_out, int *ind_count_out) {
    int max_verts = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 24;
    int max_inds = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 36;
    float *verts = (float *) malloc ((size_t) max_verts * 9 * sizeof (float));
    unsigned int *inds = (unsigned int *) malloc ((size_t) max_inds * sizeof (unsigned int));
    int vc = 0;
    int ic = 0;

    int base_x = c->cx * CHUNK_SIZE;
    int base_y = c->cy * CHUNK_SIZE;
    int base_z = c->cz * CHUNK_SIZE;

    for (int ly = 0; ly < CHUNK_SIZE; ly++) {
        for (int lz = 0; lz < CHUNK_SIZE; lz++) {
            for (int lx = 0; lx < CHUNK_SIZE; lx++) {
                int bid = chunk_get (c, lx, ly, lz);
                if (bid == BLOCK_AIR) {continue;}
                const block_type *bt = block_type_get (bid);
                if (!bt->solid) {continue;}

                float cr = bt->colour_r;
                float cg = bt->colour_g;
                float cb = bt->colour_b;

                int wx = base_x + lx;
                int wy = base_y + ly;
                int wz = base_z + lz;

                for (int face = 0; face < 6; face++) {
                    int nx = (int) face_norms [face][0];
                    int ny = (int) face_norms [face][1];
                    int nz = (int) face_norms [face][2];
                    int neighbor = world_get (w, wx + nx, wy + ny, wz + nz);
                    const block_type *nbt = block_type_get (neighbor);
                    if (nbt->solid && !nbt->transparent) {continue;}

                    unsigned int base_vert = (unsigned int) vc / 9;
                    for (int v = 0; v < 4; v++) {
                        verts [vc++] = (float) wx + face_verts_correct [face][v][0];
                        verts [vc++] = (float) wy + face_verts_correct [face][v][1];
                        verts [vc++] = (float) wz + face_verts_correct [face][v][2];
                        verts [vc++] = face_norms [face][0];
                        verts [vc++] = face_norms [face][1];
                        verts [vc++] = face_norms [face][2];
                        verts [vc++] = cr;
                        verts [vc++] = cg;
                        verts [vc++] = cb;
                    }
                    for (int idx = 0; idx < 6; idx++) {
                        inds [ic++] = base_vert + face_indices [face][idx];
                    }
                }
            }
        }
    }

    *verts_out = verts;
    *inds_out = inds;
    *vert_count_out = vc / 9;
    *ind_count_out = ic;
}

void block_render_world (const game_world *w,
                         const float *projection_flat,
                         const float *view_flat,
                         float cam_x, float cam_y, float cam_z) {
    if (!block_shader) {return;}

    glUseProgram (block_shader);
    glUniformMatrix4fv (block_proj_loc, 1, GL_FALSE, projection_flat);
    glUniformMatrix4fv (block_view_loc, 1, GL_FALSE, view_flat);
    glUniform3f (block_cam_loc, cam_x, cam_y, cam_z);
    glUniform3f (block_light_loc, 100.0f, 200.0f, 50.0f);

    for (int cy = 0; cy < WORLD_CHUNKS_Y; cy++) {
        for (int cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            for (int cx = 0; cx < WORLD_CHUNKS_X; cx++) {
                chunk *c = (chunk *) &w->chunks [(cy * WORLD_CHUNKS_Z + cz) * WORLD_CHUNKS_X + cx];

                if (c->dirty) {
                    float *verts = NULL;
                    unsigned int *inds = NULL;
                    int vc = 0, ic = 0;
                    build_chunk_mesh (w, c, &verts, &inds, &vc, &ic);

                    if (c->vao) {glDeleteVertexArrays (1, &c->vao);}
                    if (c->vbo) {glDeleteBuffers (1, &c->vbo);}
                    if (c->ebo) {glDeleteBuffers (1, &c->ebo);}

                    if (ic == 0) {
                        c->vao = 0;
                        c->vbo = 0;
                        c->ebo = 0;
                        c->index_count = 0;
                        c->dirty = false;
                        free (verts);
                        free (inds);
                        continue;
                    }

                    glGenVertexArrays (1, &c->vao);
                    glGenBuffers (1, &c->vbo);
                    glGenBuffers (1, &c->ebo);

                    glBindVertexArray (c->vao);

                    glBindBuffer (GL_ARRAY_BUFFER, c->vbo);
                    glBufferData (GL_ARRAY_BUFFER, vc * 9 * sizeof (float), verts, GL_STATIC_DRAW);

                    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, c->ebo);
                    glBufferData (GL_ELEMENT_ARRAY_BUFFER, ic * sizeof (unsigned int), inds, GL_STATIC_DRAW);

                    /* position */
                    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof (float), (void *) 0);
                    glEnableVertexAttribArray (0);
                    /* normal */
                    glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof (float), (void *) (3 * sizeof (float)));
                    glEnableVertexAttribArray (1);
                    /* color */
                    glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof (float), (void *) (6 * sizeof (float)));
                    glEnableVertexAttribArray (2);

                    glBindVertexArray (0);

                    c->index_count = ic;
                    c->dirty = false;

                    free (verts);
                    free (inds);
                }

                if (c->vao && c->index_count > 0) {
                    glBindVertexArray (c->vao);
                    glDrawElements (GL_TRIANGLES, c->index_count, GL_UNSIGNED_INT, 0);
                    glBindVertexArray (0);
                }
            }
        }
    }
}

void block_render_cleanup (void) {
    if (block_shader) {
        glDeleteProgram (block_shader);
        block_shader = 0;
    }
}
