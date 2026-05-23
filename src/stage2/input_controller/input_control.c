#include "input_control.h"
#include "../camera/camera.h"
#include "../../stage3/input_extension/mouse_lock.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdbool.h>
extern camera main_camera_fov;
extern input_status main_inputs;
void initialise_input (input_status *input_state) {
    // Keyboard Movement Inputs
    input_state -> w_key_pressed = false;
    input_state -> a_key_pressed = false;
    input_state -> s_key_pressed = false;
    input_state -> d_key_pressed = false;
    input_state -> space_key_pressed = false;
    input_state -> shift_key_pressed = false;
    input_state -> escape_key_pressed = false;
    input_state -> f_key_pressed = false;
    // Object manipulation
    input_state -> i_key_pressed = false;
    input_state -> j_key_pressed = false;
    input_state -> k_key_pressed = false;
    input_state -> l_key_pressed = false;
    // Menu logic
    input_state -> is_menu_open = false;
    input_state -> menu_1_pressed = false;
    input_state -> menu_2_pressed = false;
    input_state -> menu_3_pressed = false;
    // Spawn Object Status
    input_state -> spawner_menu_level = 0;
    input_state -> velocity_menu_level = 0;
    input_state -> object_menu_level = 0;
    input_state -> current_spawn_type = 0; // 0: Sphere, 1: Cube
    input_state -> up_arrow_pressed = false;
    input_state -> down_arrow_pressed = false;
    input_state -> left_arrow_pressed = false;
    input_state -> right_arrow_pressed = false;
    input_state -> enter_key_pressed = false;
    input_state -> e_key_pressed = false;
    // Mouse
    input_state -> is_mouse_locked = false;
    input_state -> left_mouse_button_clicked = false;
    input_state -> right_mouse_button_clicked = false;
    input_state -> middle_mouse_button_clicked = false;
    input_state -> mouse_delta_x = 0.0f;
    input_state -> mouse_delta_y = 0.0f;
} gboolean on_keypress (GtkWidget *widget, GdkEventKey *event, gpointer user_data_stored) {
    (void) widget;
    input_status *input_state = &main_inputs;
    if (event -> keyval == GDK_KEY_w) {input_state -> w_key_pressed = true;}
    if (event -> keyval == GDK_KEY_a) {input_state -> a_key_pressed = true;}
    if (event -> keyval == GDK_KEY_s) {input_state -> s_key_pressed = true;}
    if (event -> keyval == GDK_KEY_d) {input_state -> d_key_pressed = true;}
    if (event -> keyval == GDK_KEY_e) {input_state -> e_key_pressed = true;}
    if (event -> keyval == GDK_KEY_f) {input_state -> f_key_pressed = true;}
    if (event -> keyval == GDK_KEY_i) {input_state -> i_key_pressed = true;}
    if (event -> keyval == GDK_KEY_j) {input_state -> j_key_pressed = true;}
    if (event -> keyval == GDK_KEY_k) {input_state -> k_key_pressed = true;}
    if (event -> keyval == GDK_KEY_l) {input_state -> l_key_pressed = true;}
    if (event -> keyval == GDK_KEY_space) { input_state -> space_key_pressed = true; }
    if ((event -> keyval == GDK_KEY_Shift_L) || (event -> keyval == GDK_KEY_Shift_R)) { input_state -> shift_key_pressed = true; }
    if (event -> keyval == GDK_KEY_Escape) { input_state -> escape_key_pressed = true; }
    if (event -> keyval == GDK_KEY_Up) { input_state -> up_arrow_pressed = true; }
    if (event -> keyval == GDK_KEY_Down) { input_state -> down_arrow_pressed = true; }
    if (event -> keyval == GDK_KEY_Left) { input_state -> left_arrow_pressed = true; }
    if (event -> keyval == GDK_KEY_Right) { input_state -> right_arrow_pressed = true; }
    if ((event -> keyval == GDK_KEY_Return) || (event -> keyval == GDK_KEY_KP_Enter)) { input_state -> enter_key_pressed = true; }
    //Legacy Menu Toggles (7, 8, 9)
    if (event -> keyval == GDK_KEY_9) {
        input_state -> spawner_menu_level = 0;
        input_state -> velocity_menu_level = 0;
        input_state -> object_menu_level = 0;
        input_state -> is_menu_open = !(input_state -> is_menu_open);
    } if (event -> keyval == GDK_KEY_8) {
        input_state -> is_menu_open = false;
        input_state -> velocity_menu_level = 0;
        input_state -> object_menu_level = 0;
        input_state -> spawner_menu_level = (input_state -> spawner_menu_level > 0) ? 0 : 1;
    } if (event -> keyval == GDK_KEY_7) {
        input_state -> is_menu_open = false;
        input_state -> spawner_menu_level = 0;
        input_state -> object_menu_level = 0;
        input_state -> velocity_menu_level = (input_state -> velocity_menu_level > 0) ? 0 : 1;
    } //Spawner Menu Logic (8)
    if (input_state -> spawner_menu_level == 1) {
        if (event -> keyval == GDK_KEY_1) { input_state -> spawner_menu_level = 2; }
    } else if (input_state -> spawner_menu_level == 2) {
        if (event -> keyval == GDK_KEY_1) { input_state -> spawner_menu_level = 3; }
        if (event -> keyval == GDK_KEY_2) { input_state -> spawner_menu_level = 4; }
    } //Velocity Menu Logic (7)
    if (input_state -> velocity_menu_level == 1) {
        if (event -> keyval == GDK_KEY_1) { input_state -> velocity_menu_level = 2; }
        if (event -> keyval == GDK_KEY_2) { input_state -> velocity_menu_level = 10; }
        if (event -> keyval == GDK_KEY_3) { input_state -> velocity_menu_level = 20; }
    } else if (input_state -> velocity_menu_level == 2) {
        if (event -> keyval == GDK_KEY_1) { input_state -> velocity_menu_level = 3; }
        if (event -> keyval == GDK_KEY_2) { input_state -> velocity_menu_level = 4; }
    } else if (input_state -> velocity_menu_level == 20) {
        if (event -> keyval == GDK_KEY_1) { input_state -> velocity_menu_level = 21; }
        if (event -> keyval == GDK_KEY_2) { input_state -> velocity_menu_level = 22; }
        if (event -> keyval == GDK_KEY_3) { input_state -> velocity_menu_level = 23; }
    } else if (input_state -> velocity_menu_level == 10) {
        if (event -> keyval == GDK_KEY_1) { input_state -> velocity_menu_level = 11; }
        if (event -> keyval == GDK_KEY_2) { input_state -> velocity_menu_level = 12; }
    } //Object Menu Logic (E)
    if (input_state -> object_menu_level == 1) {
        if (event -> keyval == GDK_KEY_1) { input_state -> object_menu_level = 2; }
        if (event -> keyval == GDK_KEY_2) { input_state -> object_menu_level = 3; }
        if (event -> keyval == GDK_KEY_3) { input_state -> object_menu_level = 4; }
        if (event -> keyval == GDK_KEY_4) { input_state -> object_menu_level = 5; }
    } if (input_state -> is_menu_open) {
        if (event -> keyval == GDK_KEY_1) { input_state -> menu_1_pressed = true; }
        if (event -> keyval == GDK_KEY_2) { input_state -> menu_2_pressed = true; }
        if (event -> keyval == GDK_KEY_3) { input_state -> menu_3_pressed = true; }
    } return FALSE;
} gboolean on_key_released (GtkWidget *widget, GdkEventKey *event, gpointer user_data_stored) {
    (void) widget;
    input_status *input_state = &main_inputs;
    if (event -> keyval == GDK_KEY_w) { input_state -> w_key_pressed = false; }
    if (event -> keyval == GDK_KEY_a) { input_state -> a_key_pressed = false; }
    if (event -> keyval == GDK_KEY_s) { input_state -> s_key_pressed = false; }
    if (event -> keyval == GDK_KEY_d) { input_state -> d_key_pressed = false; }
    if (event -> keyval == GDK_KEY_i) { input_state -> i_key_pressed = false; }
    if (event -> keyval == GDK_KEY_j) { input_state -> j_key_pressed = false; }
    if (event -> keyval == GDK_KEY_k) { input_state -> k_key_pressed = false; }
    if (event -> keyval == GDK_KEY_l) { input_state -> l_key_pressed = false; }
    if ((event -> keyval == GDK_KEY_Shift_L) || (event -> keyval == GDK_KEY_Shift_R)) {input_state -> shift_key_pressed = false;}
    if (event -> keyval == GDK_KEY_space) {input_state -> space_key_pressed = false;}
    return FALSE;
} gboolean on_mouse_movements (GtkWidget *widget, GdkEventMotion *event, gpointer user_data_stored) {
    (void) user_data_stored;
    input_status *input_state = &main_inputs;
    if (input_state -> is_mouse_locked) {
        int width = gtk_widget_get_allocated_width (widget);
        int height = gtk_widget_get_allocated_height (widget);
        int centerX = width / 2;
        int centerY = height / 2;
        int currentX = (int) event -> x;
        int currentY = (int) event -> y;
        if ((currentX == centerX) && (currentY == centerY)) {return FALSE;}
        input_state -> mouse_delta_x = (float) (currentX - centerX);
        input_state -> mouse_delta_y = (float) (currentY - centerY);
        mouse_lock_reset_centre (widget);
    } return FALSE;
} gboolean on_button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data_stored) {
    (void) widget;
    input_status *input_state = &main_inputs;
    if (event -> button == 1) { input_state -> left_mouse_button_clicked = true; }
    if (event -> button == 2) { input_state -> middle_mouse_button_clicked = true; }
    if (event -> button == 3) { input_state -> right_mouse_button_clicked = true; }
    if (!input_state -> is_mouse_locked) {
        mouse_lock_enable (gtk_widget_get_toplevel (widget));
        input_state -> is_mouse_locked = true;
    } return FALSE;
} gboolean on_button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data_stored) {
    (void) widget;
    input_status *input_state = &main_inputs;
    if (event -> button == 1) {input_state -> left_mouse_button_clicked = false;}
    if (event -> button == 2) {input_state -> middle_mouse_button_clicked = false;}
    if (event -> button == 3) {input_state -> right_mouse_button_clicked = false;}
    return FALSE;
} gboolean on_focus_out (GtkWidget *widget, GdkEventFocus *event, gpointer user_data_stored) {
    (void) event;
    input_status *input_state = &main_inputs;
    mouse_lock_disable (gtk_widget_get_toplevel (widget));
    input_state -> is_mouse_locked = false;
    initialise_input (input_state); //Reset all states on focus loss
    return FALSE;
}
