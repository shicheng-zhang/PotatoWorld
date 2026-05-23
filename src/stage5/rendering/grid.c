#include "grid.h"
#include <epoxy/gl_generated.h>
#include <stdlib.h>
void grid_init (grid_mesh *grid_mesh_object, int half_extent, int cell_spacing) {
    (void) cell_spacing;
    float spacing = 1.0f;
    int line_count_per_axis = (int)(2 * half_extent / spacing) + 1;
    int line_vertices = line_count_per_axis * 2 * 2;
    int total_vertices = 6 + line_vertices; // 6 for floor plane, rest for lines
    float *vertex_data = malloc (total_vertices * 3 * sizeof (float));
    int v_idx = 0;
    // Floor Plane (2 triangles)
    vertex_data [v_idx++] = -half_extent; vertex_data [v_idx++] = 0.0f; vertex_data [v_idx++] = -half_extent;
    vertex_data [v_idx++] =  half_extent; vertex_data [v_idx++] = 0.0f; vertex_data [v_idx++] = -half_extent;
    vertex_data [v_idx++] =  half_extent; vertex_data [v_idx++] = 0.0f; vertex_data [v_idx++] =  half_extent;
    vertex_data [v_idx++] = -half_extent; vertex_data [v_idx++] = 0.0f; vertex_data [v_idx++] = -half_extent;
    vertex_data [v_idx++] =  half_extent; vertex_data [v_idx++] = 0.0f; vertex_data [v_idx++] =  half_extent;
    vertex_data [v_idx++] = -half_extent; vertex_data [v_idx++] = 0.0f; vertex_data [v_idx++] =  half_extent;
    // X lines
    for (int i = 0; i < line_count_per_axis; i++) {
        float z = -half_extent + i * spacing;
        vertex_data [v_idx++] = -half_extent; vertex_data [v_idx++] = 0.001f; vertex_data [v_idx++] = z;
        vertex_data [v_idx++] = half_extent;  vertex_data [v_idx++] = 0.001f; vertex_data [v_idx++] = z;
    } // Z lines
    for (int i = 0; i < line_count_per_axis; i++) {
        float x = -half_extent + i * spacing;
        vertex_data [v_idx++] = x; vertex_data [v_idx++] = 0.001f; vertex_data [v_idx++] = -half_extent;
        vertex_data [v_idx++] = x; vertex_data [v_idx++] = 0.001f; vertex_data [v_idx++] = half_extent;
    } grid_mesh_object -> line_vertex_count = total_vertices;
    glGenVertexArrays (1, &grid_mesh_object -> vertex_array_object);
    glGenBuffers (1, &grid_mesh_object -> vertex_buffer_object);
    glBindVertexArray (grid_mesh_object -> vertex_array_object);
    glBindBuffer (GL_ARRAY_BUFFER, grid_mesh_object -> vertex_buffer_object);
    glBufferData (GL_ARRAY_BUFFER, total_vertices * 3 * sizeof (float), vertex_data, GL_STATIC_DRAW);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (float), (void*) 0);
    glEnableVertexAttribArray (0);
    glBindVertexArray (0);
    free (vertex_data);
} void grid_render (grid_mesh *grid_mesh_object, GLuint shader_program, math4 view_matrix, math4 projection_matrix) {
    glUseProgram (shader_program);
    float view_matrix_flat_array [16], projection_matrix_flat_array [16];
    math4_to_flat_array (view_matrix, view_matrix_flat_array);
    math4_to_flat_array (projection_matrix, projection_matrix_flat_array);
    glUniformMatrix4fv (glGetUniformLocation (shader_program, "viewframe"), 1, GL_FALSE, view_matrix_flat_array);
    glUniformMatrix4fv (glGetUniformLocation (shader_program, "projection"), 1, GL_FALSE, projection_matrix_flat_array);
    math4 model_matrix = math4_identity ();
    float model_matrix_flat_array [16];
    math4_to_flat_array (model_matrix, model_matrix_flat_array);
    glUniformMatrix4fv (glGetUniformLocation (shader_program, "model"), 1, GL_FALSE, model_matrix_flat_array);
    math3 identity_normal_matrix = math3_identity ();
    float normal_matrix_flat_array [9];
    for (int row_index = 0; row_index < 3; row_index++) {
        for (int column_index = 0; column_index < 3; column_index++) {normal_matrix_flat_array [row_index * 3 + column_index] = identity_normal_matrix.matrix [row_index][column_index];}
    } glUniformMatrix3fv (glGetUniformLocation (shader_program, "normal_matrix"), 1, GL_FALSE, normal_matrix_flat_array);
    glBindVertexArray (grid_mesh_object -> vertex_array_object);
    // 1. Render Dark Green Floor Plane
    glUniform3f (glGetUniformLocation (shader_program, "object_colour"), 0.05f, 0.25f, 0.05f);
    glVertexAttrib3f (1, 0.0f, 1.0f, 0.0f);
    glDrawArrays (GL_TRIANGLES, 0, 6);
    // 2. Render Light Gray Grid Lines (slightly above to avoid Z-fighting)
    glUniform3f (glGetUniformLocation (shader_program, "object_colour"), 0.8f, 0.8f, 0.8f);
    glDrawArrays (GL_LINES, 6, grid_mesh_object -> line_vertex_count - 6);
    glBindVertexArray (0);
}
