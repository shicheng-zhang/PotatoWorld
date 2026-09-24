#include "../mpe_engine.h"
#include "../game/game_init.h"
#include "../game/block_render.h"
#include "../game/raycast.h"
#include "../game/block.h"
#include "../game/player.h"
#include "../game/item_registry.h"
#include <epoxy/gl.h>
#include <epoxy/gl_generated.h>
#include <sys/types.h>
#include <stdlib.h>
extern camera main_camera_fov;
extern rigidbody *obj_per_scene;
extern int object_count;
extern mesh cube_mesh;
static GLuint instanced_shader_program = 0;
static GLuint utility_shader_program = 0;
static struct {
    GLint projection_matrix_location;
    GLint view_matrix_location;
    GLint camera_position_location;
    GLint light_position_location;
} instanced_uniforms;
static struct {
    GLint projection_matrix_location;
    GLint view_matrix_location;
    GLint model_matrix_location;
    GLint normal_matrix_location;
    GLint object_colour_location;
    GLint camera_position_location;
    GLint light_position_location;
} utility_uniforms;
mesh sphere_mesh;
static int render_init_status = 0;
static grid_mesh main_grid;
static float *sphere_instances = NULL;
static float *cube_instances = NULL;
void render_init () {
    if (render_init_status) {return;}
    instanced_shader_program = create_shader_program ("render/shaders/vertex_shader.glsl", "render/shaders/fragment_shader.glsl");
    utility_shader_program = create_shader_program ("render/shaders/utility_vertex.glsl", "render/shaders/utility_fragment.glsl");
    if ((instanced_shader_program == 0) || (utility_shader_program == 0)) {
    fprintf (stderr, "RENDER INIT FAILED: shader program creation failed (instanced=%u, utility=%u)\n", instanced_shader_program, utility_shader_program);
    render_init_status = -1;
    return;
    }
    instanced_uniforms.projection_matrix_location = glGetUniformLocation (instanced_shader_program, "projection");
    instanced_uniforms.view_matrix_location = glGetUniformLocation (instanced_shader_program, "viewframe");
    instanced_uniforms.camera_position_location = glGetUniformLocation (instanced_shader_program, "camera_position");
    instanced_uniforms.light_position_location = glGetUniformLocation (instanced_shader_program, "light_position");
    utility_uniforms.projection_matrix_location = glGetUniformLocation (utility_shader_program, "projection");
    utility_uniforms.view_matrix_location = glGetUniformLocation (utility_shader_program, "viewframe");
    utility_uniforms.model_matrix_location = glGetUniformLocation (utility_shader_program, "model");
    utility_uniforms.normal_matrix_location = glGetUniformLocation (utility_shader_program, "normal_matrix");
    utility_uniforms.object_colour_location = glGetUniformLocation (utility_shader_program, "object_colour");
    utility_uniforms.camera_position_location = glGetUniformLocation (utility_shader_program, "camera_position");
    utility_uniforms.light_position_location = glGetUniformLocation (utility_shader_program, "light_position");
    grid_init (&main_grid, 250, 5);
    init_sm_system (&sphere_mesh, 32, 32);
    cube_meshing_init ();
    sphere_instances = malloc (MPE_MAX_BODIES * 19 * sizeof (float));
    cube_instances = malloc (MPE_MAX_BODIES * 19 * sizeof (float));
    render_init_status = 1;
} void render_scene_current (int widget_width, int widget_height) {
    render_init ();
    if (render_init_status < 0) {
    glViewport (0, 0, widget_width, widget_height);
    glClearColor (0.5f, 0.0f, 0.0f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return;
    }
    glViewport (0, 0, widget_width, widget_height);
    // day/night cycle: 60 sec per day
    static float day_time = 0.0f;
    day_time += main_timer.delta_time * 0.08f;
    float sun_angle = day_time;
    float sun_x = cosf(sun_angle) * 180.0f;
    float sun_y = sinf(sun_angle) * 180.0f + 80.0f;
    float sun_z = sinf(sun_angle*0.7f) * 80.0f;
    if (sun_y < -50) sun_y = -50;
    float t = (sinf(sun_angle)*0.5f + 0.5f);
    t = powf(t, 0.9f);
    float sky_r = 0.02f + t * (0.53f - 0.02f);
    float sky_g = 0.06f + t * (0.81f - 0.06f);
    float sky_b = 0.15f + t * (0.92f - 0.15f);
    // torch-like ambient at night
    if (t < 0.2f) { sky_r += 0.03f; sky_g += 0.02f; }
    glClearColor (sky_r, sky_g, sky_b, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float window_aspect_ratio = (float) (widget_width) / (float) (widget_height);
    math4 projection_matrix = math4_perspective_fov (degrad * 45.0f, window_aspect_ratio, 0.1f, 1000.0f);
    float projection_matrix_flat_array [16];
    math4_to_flat_array (projection_matrix, projection_matrix_flat_array);
    math4 view_matrix = math4_look_view (main_camera_fov.position, main_camera_fov.forward_vector, main_camera_fov.vertical_vector);
    float view_matrix_flat_array [16];
    math4_to_flat_array (view_matrix, view_matrix_flat_array);
    /* grid removed — terrain replaces the floor */
    int sphere_inst_count = 0;
    int cube_inst_count = 0;
    for (int object_index = 0; object_index < object_count; object_index++) {
        rigidbody *rigid_body = &obj_per_scene [object_index];
        math4 translation_matrix = math4_translation (rigid_body -> position);
        math4 rotation_matrix = vector4_to_math4 (rigid_body -> orientation);
        math4 scale_matrix;
        if (rigid_body -> type == object_sphere) {scale_matrix = math4_scaling ((vector3) {rigid_body -> radius, rigid_body -> radius, rigid_body -> radius});}
        else {scale_matrix = math4_scaling (rigid_body -> half_extensions);}
        math4 model_matrix = math4_multiplication (translation_matrix, math4_multiplication (rotation_matrix, scale_matrix));
        float *target_array;
        int *target_count;
        if (rigid_body -> type == object_sphere) {target_array = sphere_instances; target_count = &sphere_inst_count;}
        else {target_array = cube_instances; target_count = &cube_inst_count;}
        if ((*target_count) < MPE_MAX_BODIES) {
            int idx = (*target_count) * 19;
            math4_to_flat_array (model_matrix, &target_array [idx]);
            target_array [idx + 16] = rigid_body -> colour.x;
            target_array [idx + 17] = rigid_body -> colour.y;
            target_array [idx + 18] = rigid_body -> colour.z;
            (*target_count)++;
        }
    } glUseProgram (instanced_shader_program);
    glUniformMatrix4fv (instanced_uniforms.projection_matrix_location, 1, GL_FALSE, projection_matrix_flat_array);
    glUniformMatrix4fv (instanced_uniforms.view_matrix_location, 1, GL_FALSE, view_matrix_flat_array);
    glUniform3f (instanced_uniforms.camera_position_location, main_camera_fov.position.x, main_camera_fov.position.y, main_camera_fov.position.z);
    glUniform3f (instanced_uniforms.light_position_location, sun_x, sun_y, sun_z);
    if (sphere_inst_count > 0) {
        glBindBuffer (GL_ARRAY_BUFFER, sphere_mesh.instance_vbo);
        glBufferSubData (GL_ARRAY_BUFFER, 0, sphere_inst_count * 19 * sizeof (float), sphere_instances);
        glBindVertexArray (sphere_mesh.vertex_array_object);
        glDrawElementsInstanced (GL_TRIANGLES, sphere_mesh.index_count, GL_UNSIGNED_INT, 0, sphere_inst_count);
    } if (cube_inst_count > 0) {
        glBindBuffer (GL_ARRAY_BUFFER, cube_mesh.instance_vbo);
        glBufferSubData (GL_ARRAY_BUFFER, 0, cube_inst_count * 19 * sizeof (float), cube_instances);
        glBindVertexArray (cube_mesh.vertex_array_object);
        glDrawElementsInstanced (GL_TRIANGLES, cube_mesh.index_count, GL_UNSIGNED_INT, 0, cube_inst_count);
    } glBindVertexArray (0);
    spring_joint_render (utility_shader_program, view_matrix, projection_matrix);
    wireframe_render_selected_object (utility_shader_program, view_matrix, projection_matrix);
    block_render_world_sun (&pw_world, projection_matrix_flat_array, view_matrix_flat_array,
                        main_camera_fov.position.x, main_camera_fov.position.y, main_camera_fov.position.z,
                        sun_x, sun_y, sun_z);
    // Hardcoded preview: ghost block at placement pos (actual render, translucent)
    {
        raycast_result pr = raycast_voxel(&pw_world, main_camera_fov.position.x, main_camera_fov.position.y, main_camera_fov.position.z,
                                          main_camera_fov.forward_vector.x, main_camera_fov.forward_vector.y, main_camera_fov.forward_vector.z, 8.0f);
        if (pr.hit) {
            int px = pr.block_x + pr.normal_x;
            int py = pr.block_y + pr.normal_y;
            int pz = pr.block_z + pr.normal_z;
            // same checks as place: not inside player, empty
            float p_min_x = main_camera_fov.position.x - 0.3f, p_max_x = main_camera_fov.position.x + 0.3f;
            float p_min_y = main_camera_fov.position.y - 1.8f, p_max_y = main_camera_fov.position.y;
            float p_min_z = main_camera_fov.position.z - 0.3f, p_max_z = main_camera_fov.position.z + 0.3f;
            bool inside_player = (px+1 > p_min_x && px < p_max_x && py+1 > p_min_y && py < p_max_y && pz+1 > p_min_z && pz < p_max_z);
            if (!inside_player && world_get(&pw_world, px,py,pz)==BLOCK_AIR) {
                rigidbody ghost;
                rigidbody_initialisation_cube(&ghost, (vector3){px+0.5f, py+0.5f, pz+0.5f}, (vector3){0.5f,0.5f,0.5f}, 1.0f);
                const block_type *bt = block_type_get(pw_selected_block);
                vector3 ghost_col = {bt->colour_r, bt->colour_g, bt->colour_b};
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDepthMask(GL_FALSE);
                ghost.colour = ghost_col;
                wireframe_render_object(utility_shader_program, view_matrix, projection_matrix, &ghost, (vector3){ghost_col.x*0.6f+0.4f, ghost_col.y*0.6f+0.4f, ghost_col.z*0.6f+0.4f});
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
            }
        }
    }
    // Block breaking animation: crack overlay on targeted block (hardcoded, highly visible)
    {
        raycast_result br = raycast_voxel(&pw_world, main_camera_fov.position.x, main_camera_fov.position.y, main_camera_fov.position.z,
                                          main_camera_fov.forward_vector.x, main_camera_fov.forward_vector.y, main_camera_fov.forward_vector.z, 8.0f);
        if (br.hit) {
            float prog = game_get_break_progress(br.block_x, br.block_y, br.block_z);
            if (prog >= 0.0f) {
                if (prog > 1.0f) prog = 1.0f;
                rigidbody crack;
                rigidbody_initialisation_cube(&crack, (vector3){br.block_x+0.5f, br.block_y+0.5f, br.block_z+0.5f}, (vector3){0.512f,0.512f,0.512f}, 1.0f);
                // crack color interpolates black -> red as prog increases, high contrast
                vector3 crack_col;
                if (prog < 0.25f) crack_col = (vector3){0.95f,0.95f,0.95f}; // white cracks
                else if (prog < 0.5f) crack_col = (vector3){1.0f,0.85f,0.3f}; // yellow
                else if (prog < 0.75f) crack_col = (vector3){1.0f,0.35f,0.15f}; // orange-red
                else crack_col = (vector3){0.05f,0.05f,0.05f}; // black heavy
                float s = 0.512f + prog*0.015f;
                crack.half_extensions = (vector3){s,s,s};
                rigidbody_update_axes(&crack);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDepthMask(GL_FALSE);
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(-2.0f, -2.0f);
                // thickness increases with progress
                glLineWidth(1.0f + prog*7.0f);
                wireframe_render_object(utility_shader_program, view_matrix, projection_matrix, &crack, crack_col);
                glLineWidth(1.0f);
                // extra inner cracks for high prog: draw 2nd smaller cube
                if (prog > 0.5f) {
                    rigidbody inner;
                    rigidbody_initialisation_cube(&inner, (vector3){br.block_x+0.5f, br.block_y+0.5f, br.block_z+0.5f}, (vector3){0.3f+prog*0.1f,0.02f,0.3f+prog*0.1f}, 1.0f);
                    inner.orientation = vector4_from_axis_with_angle((vector3){0,1,0}, prog*6.28f);
                    rigidbody_update_axes(&inner);
                    wireframe_render_object(utility_shader_program, view_matrix, projection_matrix, &inner, (vector3){0,0,0});
                }
                glDisable(GL_POLYGON_OFFSET_FILL);
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
            } else if (br.hit) {
                // show subtle outline when looking at block but not breaking (for feedback)
                // only in survival, faint white outline
                if (g_player.stats.game_mode != GAME_MODE_CREATIVE) {
                    // do not draw if not breaking, ghost preview already handles
                }
            }
        }
    }
    // Hardcoded in-hand pickaxe render (actual 3D object as per request)
    {
        int hand_item = player_get_item_in_hand();
        const item_type *it = item_type_get(hand_item);
        if (it && it->tool_type != TOOL_NONE && it->tool_type != TOOL_SHEARS) {
            vector3 hand_pos = vector3_addition(main_camera_fov.position, vector3_scaling(main_camera_fov.forward_vector, 0.55f));
            hand_pos = vector3_addition(hand_pos, vector3_scaling(main_camera_fov.side_vector, 0.32f));
            hand_pos = vector3_addition(hand_pos, vector3_scaling(main_camera_fov.vertical_vector, -0.28f));
            // handle
            rigidbody handle;
            rigidbody_initialisation_cube(&handle, hand_pos, (vector3){0.03f,0.03f,0.28f}, 1);
            // orient handle along forward
            handle.orientation = main_camera_fov.forward_vector.x < 0 ? vector4_from_axis_with_angle((vector3){0,1,0}, 0.4f) : vector4_identity();
            rigidbody_update_axes(&handle);
            vector3 handle_col = {0.55f,0.38f,0.20f}; // wood
            if(it->tool_tier==1) handle_col=(vector3){0.55f,0.55f,0.55f};
            else if(it->tool_tier>=2) handle_col=(vector3){0.78f,0.78f,0.80f};
            // head
            vector3 head_pos = vector3_addition(hand_pos, vector3_scaling(main_camera_fov.forward_vector, 0.28f));
            head_pos = vector3_addition(head_pos, vector3_scaling(main_camera_fov.vertical_vector, 0.06f));
            rigidbody head;
            rigidbody_initialisation_cube(&head, head_pos, (vector3){0.18f,0.04f,0.04f}, 1);
            head.orientation = handle.orientation;
            rigidbody_update_axes(&head);
            vector3 head_col = {0.6f,0.6f,0.6f};
            if(it->tool_tier==0) head_col=(vector3){0.60f,0.45f,0.25f};
            else if(it->tool_tier==2) head_col=(vector3){0.78f,0.78f,0.80f};
            else if(it->tool_tier==3) head_col=(vector3){0.35f,0.90f,0.95f};
            glDisable(GL_DEPTH_TEST);
            wireframe_render_object(utility_shader_program, view_matrix, projection_matrix, &handle, handle_col);
            wireframe_render_object(utility_shader_program, view_matrix, projection_matrix, &head, head_col);
            glEnable(GL_DEPTH_TEST);
            // solid cubes for actual render (hardcoded)
            {
                math4 mod = math4_multiplication(math4_translation(handle.position), math4_multiplication(vector4_to_math4(handle.orientation), math4_scaling(handle.half_extensions)));
                float flat[16]; math4_to_flat_array(mod, flat);
                float view_flat2[16], proj_flat2[16];
                math4_to_flat_array(view_matrix, view_flat2);
                math4_to_flat_array(projection_matrix, proj_flat2);
                glUseProgram(utility_shader_program);
                glUniformMatrix4fv(utility_uniforms.view_matrix_location,1,GL_FALSE,view_flat2);
                glUniformMatrix4fv(utility_uniforms.projection_matrix_location,1,GL_FALSE,proj_flat2);
                glUniformMatrix4fv(utility_uniforms.model_matrix_location,1,GL_FALSE,flat);
                math3 norm = math3_identity();
                float nf[9]; for(int r=0;r<3;r++) for(int c=0;c<3;c++) nf[r*3+c]=norm.matrix[r][c];
                glUniformMatrix3fv(utility_uniforms.normal_matrix_location,1,GL_FALSE,nf);
                glUniform3f(utility_uniforms.object_colour_location,handle_col.x,handle_col.y,handle_col.z);
                glUniform3f(utility_uniforms.camera_position_location, main_camera_fov.position.x,main_camera_fov.position.y,main_camera_fov.position.z);
                glUniform3f(utility_uniforms.light_position_location,sun_x,sun_y,sun_z);
                glBindVertexArray(cube_mesh.vertex_array_object);
                glDrawElements(GL_TRIANGLES, cube_mesh.index_count, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
            {
                math4 mod = math4_multiplication(math4_translation(head.position), math4_multiplication(vector4_to_math4(head.orientation), math4_scaling(head.half_extensions)));
                float flat[16]; math4_to_flat_array(mod, flat);
                float view_flat2[16], proj_flat2[16];
                math4_to_flat_array(view_matrix, view_flat2);
                math4_to_flat_array(projection_matrix, proj_flat2);
                glUseProgram(utility_shader_program);
                glUniformMatrix4fv(utility_uniforms.view_matrix_location,1,GL_FALSE,view_flat2);
                glUniformMatrix4fv(utility_uniforms.projection_matrix_location,1,GL_FALSE,proj_flat2);
                glUniformMatrix4fv(utility_uniforms.model_matrix_location,1,GL_FALSE,flat);
                math3 norm = math3_identity();
                float nf[9]; for(int r=0;r<3;r++) for(int c=0;c<3;c++) nf[r*3+c]=norm.matrix[r][c];
                glUniformMatrix3fv(utility_uniforms.normal_matrix_location,1,GL_FALSE,nf);
                glUniform3f(utility_uniforms.object_colour_location,head_col.x,head_col.y,head_col.z);
                glUniform3f(utility_uniforms.camera_position_location, main_camera_fov.position.x,main_camera_fov.position.y,main_camera_fov.position.z);
                glUniform3f(utility_uniforms.light_position_location,sun_x,sun_y,sun_z);
                glBindVertexArray(cube_mesh.vertex_array_object);
                glDrawElements(GL_TRIANGLES, cube_mesh.index_count, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        } else if (it && it->placeable) {
            // if holding block, show small block in hand
            vector3 hand_pos = vector3_addition(main_camera_fov.position, vector3_scaling(main_camera_fov.forward_vector, 0.6f));
            hand_pos = vector3_addition(hand_pos, vector3_scaling(main_camera_fov.side_vector, 0.30f));
            hand_pos = vector3_addition(hand_pos, vector3_scaling(main_camera_fov.vertical_vector, -0.30f));
            rigidbody blk;
            const block_type *bt = block_type_get(it->block_id);
            rigidbody_initialisation_cube(&blk, hand_pos, (vector3){0.12f,0.12f,0.12f}, 1);
            blk.colour = (vector3){bt->colour_r, bt->colour_g, bt->colour_b};
            // simple spin
            blk.orientation = vector4_from_axis_with_angle((vector3){0,1,0}, day_time*1.2f);
            rigidbody_update_axes(&blk);
            glDisable(GL_DEPTH_TEST);
            wireframe_render_object(utility_shader_program, view_matrix, projection_matrix, &blk, blk.colour);
            glEnable(GL_DEPTH_TEST);
        }
    }
}
