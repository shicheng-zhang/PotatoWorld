#version 330 core
out vec4 fragment_color;
in vec3 normal; //Received Data from vertex shader
in vec3 fragment_position; //Received Data from vertex shader
in vec3 local_position; //Local Position of Object
uniform vec3 light_position; //Position of light source
uniform vec3 object_colour; //color of the object rendered
uniform vec3 camera_position;
uniform int is_cube;
uniform float breaking_effect;
void main () {
    //Ambient Lighting Amount (Increased for visibility)
    float ambient_strength = 0.6f;
    vec3 ambient_light = ambient_strength * vec3 (1.0f, 1.0f, 1.0f);
    //Diffuse Lighting (Makes objects seem 3D)
    vec3 normalise = normalize (normal);
    vec3 light_direction = normalize (light_position - fragment_position);
    //Dot Product to see how much light impacts the surface of the object
    //By angle (0 deg, all light, max bright (1.0f)), (90 deg, no light, min bright (0.0f))
    float difference = max (dot (normalise, light_direction), 0.0f);
    vec3 diffusion = difference * vec3 (1.0f, 1.0f, 1.0f);
    //3D diffusion Lighting (Increased Specular)
    float specular_lighting_intensity_coefficient = 0.8f;
    vec3 camera_to_fragment_view_direction_vector = normalize (camera_position - fragment_position);
    vec3 light_reflection_direction_vector = reflect (-light_direction, normalise);
    float specular_exponent_factor = pow (max (dot (camera_to_fragment_view_direction_vector, light_reflection_direction_vector), 0.0f), 32.0f);
    vec3 specular_lighting_result_colour = specular_lighting_intensity_coefficient * specular_exponent_factor * vec3 (1.0f, 1.0f, 1.0f);
    vec3 final_calculated_pixel_colour = (ambient_light + diffusion + specular_lighting_result_colour) * object_colour;
    // Breaking progress effect: moving jagged lines (cracks)
    if (breaking_effect > 0.01) {
        // Create a repeating pattern of lines based on local position
        float scale = 8.0;
        vec3 p = local_position * scale;
        // Jagged line pattern using sin/frac
        float pattern = sin(p.x + p.y + p.z) * sin(p.x - p.y) * sin(p.y - p.z);
        float line_threshold = 1.0 - breaking_effect;
        float crack_mask = smoothstep(line_threshold - 0.1, line_threshold, pattern);
        // Darken the color where cracks appear
        final_calculated_pixel_colour = mix(final_calculated_pixel_colour, vec3(0.1), crack_mask * 0.7);
    }
    // Permanent sharp black borders along all 12 edges of the cube
    if (is_cube == 1) {
        float border_thickness = 0.04;
        vec3 near_edge = step (1.0 - border_thickness, abs (local_position));
        // A point is on an edge if it is near the boundary on at least two axes
        float edge_mask = max (near_edge.x * near_edge.y, max (near_edge.y * near_edge.z, near_edge.z * near_edge.x));
        final_calculated_pixel_colour = mix (final_calculated_pixel_colour, vec3 (0.05), edge_mask);
    } // Axis rings — painted onto local midplanes so they rotate with the object
    // X ring (YZ plane, x = 0): red — X-axis rotation
    // Y ring (XZ plane, y = 0): green — Y-axis rotation
    // Z ring (XY plane, z = 0): blue — Z-axis rotation
    float stripe_width = 0.06;
    float x_ring = 1.0 - smoothstep (0.0, stripe_width, abs (local_position.x));
    float y_ring = 1.0 - smoothstep (0.0, stripe_width, abs (local_position.y));
    float z_ring = 1.0 - smoothstep (0.0, stripe_width, abs (local_position.z));
    vec3 ring_colour = vec3 (0.0);
    ring_colour += x_ring * vec3 (0.9, 0.15, 0.15);
    ring_colour += y_ring * vec3 (0.15, 0.9, 0.15);
    ring_colour += z_ring * vec3 (0.15, 0.35, 1.0);
    float ring_mask = clamp (x_ring + y_ring + z_ring, 0.0, 1.0);
    final_calculated_pixel_colour = mix (final_calculated_pixel_colour, ring_colour, ring_mask * 0.85);
    fragment_color = vec4 (final_calculated_pixel_colour, 1.0f);
}
