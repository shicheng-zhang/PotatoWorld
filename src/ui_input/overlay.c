#include "../mpe_engine.h"
#include "overlay.h"
#include "object_spawner.h"
#include "../game/block.h"
#include "../game/game_init.h"
#include "../game/player.h"
#include "../game/item_registry.h"
#include "../game/raycast.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static GtkWidget *debug_information_label = NULL;
static GtkWidget *crosshair_label = NULL;
static GtkWidget *menu_label = NULL;
static GtkWidget *spawner_menu_label = NULL;
static GtkWidget *velocity_menu_label = NULL;
static GtkWidget *object_menu_label = NULL;
static GtkWidget *test_menu_label = NULL;
// Hardcoded HUD renders: hearts + hunger + hotbar graphical
static GtkWidget *hearts_box = NULL;
static GtkWidget *hearts_labels[10] = {NULL};
static GtkWidget *hunger_box = NULL;
static GtkWidget *hunger_labels[10] = {NULL};
static GtkWidget *hotbar_box = NULL;
static GtkWidget *hotbar_slots[9] = {NULL};
extern input_status main_inputs;
extern camera main_camera_fov;
extern float world_gravity_y;
extern float world_drag_coefficient;
extern float world_surface_friction_static;
extern float world_surface_friction_kinetic;
extern rigidbody *obj_per_scene;
extern int object_count;
extern int selected_object;
extern float variable_change_rate;
extern float jump_height;


static void overlay_append_overflow_text (char *buffer, size_t buffer_size) {
    /* A3_PATCH_14_OVERFLOW_VISIBILITY */
    int node_overflow_count = broadphase_get_node_overflow_count ();
    int pair_overflow_count = broadphase_get_pair_overflow_count ();
/* MPE_TASK_11_OVERLAY_LARGE_CLAMP_BEGIN */
int large_clamp_count = broadphase_get_large_object_clamp_count ();
if (large_clamp_count > 0) {
size_t large_clamp_current_length = strlen (buffer);
if (large_clamp_current_length < buffer_size) {
snprintf (buffer + large_clamp_current_length, buffer_size - large_clamp_current_length,
" | BP large clamps:%d", large_clamp_count);
}
}
/* MPE_TASK_11_OVERLAY_LARGE_CLAMP_END */
/* MPE_TASK_10_OVERLAY_DEDUPE_BEGIN */
int dedupe_overflow_count = broadphase_get_pair_dedupe_overflow_count ();
if (dedupe_overflow_count > 0) {
size_t dedupe_current_length = strlen (buffer);
if (dedupe_current_length < buffer_size) {
snprintf (buffer + dedupe_current_length, buffer_size - dedupe_current_length,
" | BP dedupe overflow:%d", dedupe_overflow_count);
}
}
/* MPE_TASK_10_OVERLAY_DEDUPE_END */
/* MPE_TASK_09_OVERLAY_MANIFOLD_OVERFLOW_BEGIN */
if (debug_last_manifold_overflow_count > 0) {
size_t manifold_current_length = strlen (buffer);
if (manifold_current_length < buffer_size) {
snprintf (buffer + manifold_current_length, buffer_size - manifold_current_length,
" | Manifold overflow:%d", debug_last_manifold_overflow_count);
}
}
/* MPE_TASK_09_OVERLAY_MANIFOLD_OVERFLOW_END */

    if ((node_overflow_count > 0) || (pair_overflow_count > 0)) {
        size_t current_length = strlen (buffer);

        if (current_length < buffer_size) {
            snprintf (buffer + current_length, buffer_size - current_length,
                      " | BP overflow N:%d P:%d",
                      node_overflow_count, pair_overflow_count);
        }
    }
}
static bool overlay_has_valid_selection (void) {
    /* A3_PATCH_05_OVERLAY_VALIDATION */
    return (selected_object >= 0) && (selected_object < object_count);
}
GtkWidget *overlay_initialise (GtkWidget *gl_drawing_area_widget) {
    //Debug Info
    GtkWidget *ui_overlay_container = gtk_overlay_new ();
    gtk_container_add (GTK_CONTAINER (ui_overlay_container), gl_drawing_area_widget);
    debug_information_label = gtk_label_new ("- Miniature Physics Engine v1.4 Alpha 3 -");
    gtk_widget_set_halign (debug_information_label, GTK_ALIGN_START);
    gtk_widget_set_valign (debug_information_label, GTK_ALIGN_START);
    gtk_overlay_add_overlay (GTK_OVERLAY (ui_overlay_container), debug_information_label);
    //Crosshair
    crosshair_label = gtk_label_new ("+");
    gtk_widget_set_halign (crosshair_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (crosshair_label, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay (GTK_OVERLAY (ui_overlay_container), crosshair_label);
    gtk_widget_show (crosshair_label);
    //Combined menu
    menu_label = gtk_label_new ("");
    gtk_widget_set_halign (menu_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (menu_label, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay (GTK_OVERLAY (ui_overlay_container), menu_label);
    gtk_widget_hide (menu_label);
    //Totoal spawner characteristics menu
    spawner_menu_label = gtk_label_new ("");
    gtk_widget_set_halign (spawner_menu_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (spawner_menu_label, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay (GTK_OVERLAY (ui_overlay_container), spawner_menu_label);
    gtk_widget_hide (spawner_menu_label);
    //Spawn Velocity Change
    velocity_menu_label = gtk_label_new ("");
    gtk_widget_set_halign (velocity_menu_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (velocity_menu_label, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay (GTK_OVERLAY (ui_overlay_container), velocity_menu_label);
    gtk_widget_hide (velocity_menu_label);
    //Object Individual Menu
    object_menu_label = gtk_label_new ("");
    gtk_widget_set_halign (object_menu_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (object_menu_label, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay (GTK_OVERLAY (ui_overlay_container), object_menu_label);
    gtk_widget_hide (object_menu_label);
    // Test menu (F6)
    test_menu_label = gtk_label_new ("");
    gtk_widget_set_halign (test_menu_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (test_menu_label, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay (GTK_OVERLAY (ui_overlay_container), test_menu_label);
    gtk_widget_hide (test_menu_label);
    // Hearts HUD (hardcoded render: 10 hearts top-left under debug label)
    hearts_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_set_halign(hearts_box, GTK_ALIGN_START);
    gtk_widget_set_valign(hearts_box, GTK_ALIGN_START);
    gtk_widget_set_margin_top(hearts_box, 28);
    gtk_widget_set_margin_start(hearts_box, 6);
    for(int i=0;i<10;i++){
        hearts_labels[i]=gtk_label_new("♥");
        gtk_box_pack_start(GTK_BOX(hearts_box), hearts_labels[i], FALSE, FALSE, 0);
        gtk_widget_show(hearts_labels[i]);
    }
    gtk_overlay_add_overlay(GTK_OVERLAY(ui_overlay_container), hearts_box);
    gtk_widget_show(hearts_box);
    // Hunger HUD (10 drumsticks below hearts)
    hunger_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_set_halign(hunger_box, GTK_ALIGN_START);
    gtk_widget_set_valign(hunger_box, GTK_ALIGN_START);
    gtk_widget_set_margin_top(hunger_box, 50);
    gtk_widget_set_margin_start(hunger_box, 6);
    for(int i=0;i<10;i++){
        hunger_labels[i]=gtk_label_new("🍗");
        gtk_box_pack_start(GTK_BOX(hunger_box), hunger_labels[i], FALSE, FALSE, 0);
        gtk_widget_show(hunger_labels[i]);
    }
    gtk_overlay_add_overlay(GTK_OVERLAY(ui_overlay_container), hunger_box);
    gtk_widget_show(hunger_box);
    // Hotbar HUD (hardcoded 9 slots bottom-center)
    hotbar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign(hotbar_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(hotbar_box, GTK_ALIGN_END);
    gtk_widget_set_margin_bottom(hotbar_box, 8);
    for(int i=0;i<9;i++){
        GtkWidget *frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
        gtk_widget_set_size_request(frame, 48, 48);
        GtkWidget *lbl = gtk_label_new(" ");
        gtk_label_set_justify(GTK_LABEL(lbl), GTK_JUSTIFY_CENTER);
        // allow markup
        gtk_container_add(GTK_CONTAINER(frame), lbl);
        gtk_widget_show(lbl);
        gtk_widget_show(frame);
        gtk_box_pack_start(GTK_BOX(hotbar_box), frame, FALSE, FALSE, 0);
        hotbar_slots[i]=lbl;
    }
    gtk_overlay_add_overlay(GTK_OVERLAY(ui_overlay_container), hotbar_box);
    gtk_widget_show(hotbar_box);
    return ui_overlay_container;
} static void overlay_append_stats_text (char *buffer, size_t buffer_size) {
    /* A3_PATCH_36_DEBUG_COUNTERS */
    size_t current_length = strlen (buffer);

    if (current_length < buffer_size) {
        snprintf (buffer + current_length, buffer_size - current_length,
                  " | Obj:%d Pairs:%d Manifolds:%d Cache H:%d M:%d",
                  debug_last_object_count,
                  debug_last_broadphase_pair_count,
                  debug_last_manifold_count,
                  contact_cache_get_hits (),
                  contact_cache_get_misses ());
/* MPE_TASK_13_OVERLAY_LONG_RUN_BEGIN */
if (long_run_validation_active) {
size_t long_run_current_length = strlen (buffer);
if (long_run_current_length < buffer_size) {
snprintf (buffer + long_run_current_length, buffer_size - long_run_current_length,
" | LR:%ds", long_run_validation_ticks_remaining / 60);
}
}
/* MPE_TASK_13_OVERLAY_LONG_RUN_END */
/* MPE_TASK_12_OVERLAY_SLEEP_BEGIN */
size_t sleep_current_length = strlen (buffer);
if (sleep_current_length < buffer_size) {
snprintf (buffer + sleep_current_length, buffer_size - sleep_current_length,
" | Sleep:%d", debug_last_sleeping_object_count);
}
/* MPE_TASK_12_OVERLAY_SLEEP_END */
    }
}

void overlay_update (void) {
    float adjustment_increment = variable_change_rate;
    if (menu_label) {
        if (main_inputs.is_menu_open) {
            gtk_label_set_text (GTK_LABEL (menu_label), "1: Save current state\n2: Load previous state\n3: Exit");
            gtk_widget_show (menu_label);
        } else {gtk_widget_hide (menu_label);}
    } if (spawner_menu_label) {
        if (main_inputs.spawner_menu_level == 0) {gtk_widget_hide (spawner_menu_label);}
        else {
            char spawner_text [512];
            if (main_inputs.spawner_menu_level == 1) {
                const char *spawn_type_text;
                if (main_inputs.current_spawn_type == 0) {spawn_type_text = "Sphere";}
                else {spawn_type_text = "Cube";}
                snprintf (spawner_text, sizeof (spawner_text), "-- Spawner Menu --\n1: Sphere\n2: Cube\n3: Current Type: %s", spawn_type_text);
            } else if (main_inputs.spawner_menu_level == 2) {snprintf (spawner_text, sizeof (spawner_text), "-- Sphere Settings --\n1: Mass\n2: Radius");}
            else if (main_inputs.spawner_menu_level == 3) {snprintf (spawner_text, sizeof (spawner_text), "-- Mass Settings --\nCurrent Mass: %.2f kg\n\nValue dialog active (step %.2f)", spawn_mass, adjustment_increment);}
            else if (main_inputs.spawner_menu_level == 4) {snprintf (spawner_text, sizeof (spawner_text), "-- Radius Settings --\nCurrent Radius: %.2f m\n\nValue dialog active (step %.2f)", spawn_radius, adjustment_increment);}
            else if (main_inputs.spawner_menu_level == 5) {snprintf (spawner_text, sizeof (spawner_text), "-- Cube Settings --\n1: Mass\n2: Size");}
            else if (main_inputs.spawner_menu_level == 6) {snprintf (spawner_text, sizeof (spawner_text), "-- Cube Mass --\nCurrent Mass: %.2f kg\n\nValue dialog active (step %.2f)", spawn_cube_mass, adjustment_increment);}
            else if (main_inputs.spawner_menu_level == 7) {snprintf (spawner_text, sizeof (spawner_text), "-- Cube Size --\nCurrent Size: %.2f m\n\nValue dialog active (step %.2f)", spawn_cube_extent, adjustment_increment);}
            else if (main_inputs.spawner_menu_level == 8) {
                const char *spawn_type_text;
                if (main_inputs.current_spawn_type == 0) {spawn_type_text = "Sphere";}
                else {spawn_type_text = "Cube";}
                snprintf (spawner_text, sizeof (spawner_text), "-- Toggle Spawn Type --\nCurrent: %s\n\nUp/Down: Toggle\nEnter: Save and Close", spawn_type_text);
            } gtk_label_set_text (GTK_LABEL (spawner_menu_label), spawner_text);
            gtk_widget_show (spawner_menu_label);
        }
    } if (velocity_menu_label) {
        if (main_inputs.velocity_menu_level == 0) {gtk_widget_hide (velocity_menu_label);}
        else {
            char velocity_text [512];
            if (main_inputs.velocity_menu_level == 1) {snprintf (velocity_text, sizeof (velocity_text), "-- User Mechanics --\n1: Spawning\n2: Viewpoint\n3: World Modification");}
            else if (main_inputs.velocity_menu_level == 2) {snprintf (velocity_text, sizeof (velocity_text), "-- Spawning Mechanics --\n1: Launch Velocity\n2: Object Friction");}
            else if (main_inputs.velocity_menu_level == 3) {snprintf (velocity_text, sizeof (velocity_text), "-- Launch Velocity --\nCurrent: %.2f m/s\n\nValue dialog active (step %.2f)", spawn_speed, adjustment_increment);}
            else if (main_inputs.velocity_menu_level == 4) {snprintf (velocity_text, sizeof (velocity_text), "-- Object Friction --\nStatic (u_s): %.2f | Kinetic (u_k): %.2f\n\nValue dialog active (step %.2f)", friction_static, friction_kinetic, adjustment_increment);}
            else if (main_inputs.velocity_menu_level == 10) {snprintf (velocity_text, sizeof (velocity_text), "-- Viewpoint Settings --\n1: Movement Speed\n2: Character Jump Height");}
            else if (main_inputs.velocity_menu_level == 11) {snprintf (velocity_text, sizeof (velocity_text), "-- Movement Speed --\nCurrent: %.2f m/s\n\nValue dialog active (step %.2f)", main_camera_fov.movement_speed, adjustment_increment);}
            else if (main_inputs.velocity_menu_level == 12) {snprintf (velocity_text, sizeof (velocity_text), "-- Jump Height --\nCurrent: %.2f m\n\nValue dialog active (step %.2f)", jump_height, adjustment_increment);}
            else if (main_inputs.velocity_menu_level == 20) {snprintf (velocity_text, sizeof (velocity_text), "-- World Modification --\n1: Gravity\n2: Air Resistance\n3: Surface Friction");}
            else if (main_inputs.velocity_menu_level == 21) {snprintf (velocity_text, sizeof (velocity_text), "-- World Gravity --\nCurrent: %.2f m/s^2\n\nValue dialog active (step %.2f)", world_gravity_y, adjustment_increment);}
            else if (main_inputs.velocity_menu_level == 22) {snprintf (velocity_text, sizeof (velocity_text), "-- Air Resistance (Drag) --\nCurrent Coeff: %.2f\n\nValue dialog active (step %.2f)", world_drag_coefficient, adjustment_increment * 0.01f);}
            else if (main_inputs.velocity_menu_level == 23) {snprintf (velocity_text, sizeof (velocity_text), "-- Surface Friction (Floor) --\nStatic (u_s): %.2f | Kinetic (u_k): %.2f\n\nValue dialog active (step %.2f)", world_surface_friction_static, world_surface_friction_kinetic, adjustment_increment);}
            gtk_label_set_text (GTK_LABEL (velocity_menu_label), velocity_text);
            gtk_widget_show (velocity_menu_label);
        }
    } if (object_menu_label) {
        /* A3_PATCH_03_OVERLAY_GUARD */
        if ((main_inputs.object_menu_level == 0) || (selected_object < 0) || (selected_object >= object_count)) {
            if (!overlay_has_valid_selection ()) {main_inputs.object_menu_level = 0;}
            gtk_widget_hide (object_menu_label);
        }
        else {
            char object_text [512];
            rigidbody *target = &obj_per_scene [selected_object];
            if (main_inputs.object_menu_level == 1) {
                const char *type_name = (target -> type == object_sphere) ? "Sphere" : "Cube";
                int len = snprintf (object_text, sizeof (object_text),
                    "-- Object %d (%s) --\n1: Mass\n2: %s\n3: Friction\n4: Immovable Toggle\n5: Mark for Joint\n",
                    selected_object, type_name, (target -> type == object_sphere) ? "Radius" : "Radius (N/A)");
                if (main_inputs.marked_joint_object_index != -1 && main_inputs.marked_joint_object_index != selected_object) {
                    snprintf (object_text + len, sizeof (object_text) - len,
                        "6: Link Joint (from Obj %d)\n7: Colour Selection", main_inputs.marked_joint_object_index);
                } else {
                    snprintf (object_text + len, sizeof (object_text) - len,
                        "6: Colour Selection");
                }
            } else if (main_inputs.object_menu_level == 2) {snprintf (object_text, sizeof (object_text), "-- Mass Adjustment --\nCurrent: %.2f kg\n\nValue dialog active (step %.2f)", target -> mass, adjustment_increment);}
            else if (main_inputs.object_menu_level == 3) {snprintf (object_text, sizeof (object_text), "-- Radius Adjustment --\nCurrent: %.2f m\n\nValue dialog active (step %.2f)", target -> radius, adjustment_increment);}
            else if (main_inputs.object_menu_level == 4) {snprintf (object_text, sizeof (object_text), "-- Friction Adjustment --\nStatic (u_s): %.2f | Kinetic (u_k): %.2f\n\nValue dialog active (step %.2f)", target -> friction_static, target -> friction_kinetic, adjustment_increment);}
            else if (main_inputs.object_menu_level == 5) {
                const char *static_status_text;
                if (target -> static_state) {static_status_text = "YES";}
                else {static_status_text = "NO";}
                snprintf (object_text, sizeof (object_text), "-- Immovable Status --\nCurrent: %s\n\nUp/Down: Toggle\nEnter: Save", static_status_text);
            } else if (main_inputs.object_menu_level == 8) {
                snprintf (object_text, sizeof (object_text),
                    "-- Preset Colours --\n1: Red\n2: Green\n3: Blue\n4: Orange\n5: Cyan\n6: Magenta\n7: Yellow\n8: White");
            } gtk_label_set_text (GTK_LABEL (object_menu_label), object_text);
            gtk_widget_show (object_menu_label);
        }
    } if (test_menu_label) {
        if (main_inputs.test_menu_level == 0) gtk_widget_hide (test_menu_label);
        else {
            char test_text[512];
            snprintf(test_text,sizeof(test_text),
                "-- Test Suites (F6) --\n"
                "1: Stability stack (old F5)\n"
                "2: Sleep/wake test (old F6)\n"
                "3: Editor torture (old F7)\n"
                "4: Spawn stress 300 (old F8)\n"
                "5: Validation report (old F9)\n"
                "6: Long-run 60s (old F10)\n"
                "ESC: Close");
            gtk_label_set_text (GTK_LABEL (test_menu_label), test_text);
            gtk_widget_show (test_menu_label);
        }
    }
    // Hardcoded HUD renders: hearts + hunger graphical
    if (hearts_box) {
        float hp = g_player.stats.health;
        for(int i=0;i<10;i++){
            float h = hp - i*2.0f;
            const char *markup;
            if (h >= 2.0f) markup = "<span foreground='#e53935' size='large'>♥</span>";
            else if (h >= 1.0f) markup = "<span foreground='#ff8a80' size='large'>♥</span>";
            else if (hp > 0) markup = "<span foreground='#424242' size='large'>♡</span>";
            else markup = "<span foreground='#212121' size='large'>♡</span>";
            if (g_player.stats.game_mode==GAME_MODE_CREATIVE) markup = "<span foreground='#64b5f6' size='large'>♥</span>";
            gtk_label_set_markup(GTK_LABEL(hearts_labels[i]), markup);
        }
        gtk_widget_show(hearts_box);
    }
    if (hunger_box) {
        float hg = g_player.stats.hunger;
        // hide hunger in creative (like vanilla)
        if (g_player.stats.game_mode==GAME_MODE_CREATIVE) {
            for(int i=0;i<10;i++) gtk_label_set_markup(GTK_LABEL(hunger_labels[i]), "<span foreground='#2e2e2e' size='large'>·</span>");
        } else {
            for(int i=0;i<10;i++){
                float h = hg - i*2.0f;
                const char *markup;
                if (h >= 2.0f) markup = "<span foreground='#8d6e63' size='large'>🍖</span>";
                else if (h >= 1.0f) markup = "<span foreground='#bcaaa4' size='large'>🍖</span>";
                else markup = "<span foreground='#424242' size='large'>♡</span>";
                gtk_label_set_markup(GTK_LABEL(hunger_labels[i]), markup);
            }
        }
        gtk_widget_show(hunger_box);
    }
    if (hotbar_box) {
        for(int i=0;i<9;i++){
            item_stack_t *st = inventory_get_slot(i);
            const char *icon = " ";
            const char *name = "";
            int cnt = 0;
            const char *bg = "#3e2723";
            if(st && st->item_id!=ITEM_AIR){
                const item_type *it=item_type_get(st->item_id);
                name = it?it->name:"?";
                cnt = st->count;
                if(it && it->tool_type==TOOL_PICKAXE) icon="⛏";
                else if(it && it->tool_type==TOOL_AXE) icon="🪓";
                else if(it && it->tool_type==TOOL_SHOVEL) icon="⛏";
                else if(it && it->is_food) icon="🍖";
                else if(it && it->placeable) {
                    if(it->block_id==BLOCK_GRASS) icon="🟩";
                    else if(it->block_id==BLOCK_DIRT) icon="🟫";
                    else if(it->block_id==BLOCK_STONE) icon="⬜";
                    else if(it->block_id==BLOCK_SAND) icon="🟨";
                    else if(it->block_id==BLOCK_OAK_LOG) icon="🟫";
                    else icon="🧱";
                } else icon="•";
                if(i==g_player.inventory.selected_hotbar_slot) bg="#ffd54f";
            } else {
                icon=" ";
                name="";
            }
            char markup[128];
            if(cnt>0) snprintf(markup,sizeof(markup),"<span background='%s' foreground='white'><b>%s</b> %s x%d</span>", bg, icon, name, cnt);
            else snprintf(markup,sizeof(markup),"<span background='%s'>   </span>", bg);
            gtk_label_set_markup(GTK_LABEL(hotbar_slots[i]), markup);
            // highlight selected with frame
            GtkWidget *frame = gtk_widget_get_parent(hotbar_slots[i]);
            if(frame){
                if(i==g_player.inventory.selected_hotbar_slot) gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
                else gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
            }
        }
        gtk_widget_show(hotbar_box);
    }
    if (!debug_information_label) {return;}
    char information_text_buffer [1024];
    char game_mode_text [32];
    // DEBUG=CREATIVE, GAME=SURVIVAL (hard-synced via F10)
    if (main_inputs.is_debug_mode_active) {snprintf (game_mode_text, sizeof (game_mode_text), "CREATIVE");}
    else {snprintf (game_mode_text, sizeof (game_mode_text), "SURVIVAL");}
    if ((selected_object < 0) || (selected_object >= object_count)) {
        // hotbar with inventory counts (text fallback)
        char hotbar[256]="";
        for(int s=0;s<9;s++){
            item_stack_t *st = inventory_get_slot(s);
            const char *nm = "air";
            int cnt=0;
            if(st && st->item_id!=ITEM_AIR){ const item_type *it=item_type_get(st->item_id); nm=it?it->name:"?"; cnt=st->count; }
            char tmp[48];
            if(s==g_player.inventory.selected_hotbar_slot) snprintf(tmp,sizeof(tmp),"[%d:%s x%d] ",s+1,nm,cnt);
            else snprintf(tmp,sizeof(tmp),"%d:%s x%d ",s+1,nm,cnt);
            strncat(hotbar, tmp, sizeof(hotbar)-strlen(hotbar)-1);
        }
        const block_type *selbt = block_type_get(pw_selected_block);
        // breaking progress for targeted block
        raycast_result r = raycast_voxel(&pw_world, main_camera_fov.position.x, main_camera_fov.position.y, main_camera_fov.position.z,
                                         main_camera_fov.forward_vector.x, main_camera_fov.forward_vector.y, main_camera_fov.forward_vector.z, 8.0f);
        float break_prog = -1.0f;
        if(r.hit) break_prog = game_get_break_progress(r.block_x, r.block_y, r.block_z);
        char break_str[32]="";
        if(break_prog>=0) snprintf(break_str,sizeof(break_str)," Breaking %.0f%%",break_prog*100);

        char health[64];
        int hearts = (int)(g_player.stats.health/2.0f);
        int hunger = (int)(g_player.stats.hunger/2.0f);
        snprintf(health,sizeof(health),"HP:%d/10 Hunger:%d/10 %s",hearts,hunger, g_player.stats.game_mode==GAME_MODE_CREATIVE?"[Creative]":"[Survival]");
        snprintf (information_text_buffer, sizeof (information_text_buffer),
                 "[%s] %s | %s | Hotbar: %s%s",
                 game_mode_text, health, selbt->name, hotbar, break_str);
        overlay_append_stats_text (information_text_buffer, sizeof (information_text_buffer));
    overlay_append_overflow_text (information_text_buffer, sizeof (information_text_buffer));
gtk_label_set_text (GTK_LABEL (debug_information_label), information_text_buffer);
        return;
    } rigidbody *selected_rigid_body = &obj_per_scene [selected_object];
    float selected_object_speed = vector3_length (selected_rigid_body -> velocity);
    const char *object_type_text;
    if (selected_rigid_body -> type == object_sphere) {object_type_text = "Sphere";}
    else {object_type_text = "Cube";}
    const char *static_status_text;
    if (selected_rigid_body -> static_state) {static_status_text = "(Static)";}
    else {static_status_text = "(Dynamic)";}
    const block_type *selbt2 = block_type_get(pw_selected_block);
    snprintf (information_text_buffer, sizeof (information_text_buffer),
            "[%s] | %s [%d] %s | Pos: (%.1f, %.1f, %.1f) | Speed: %.2f | Block: %s | E: Menu F: Imp",
            game_mode_text,
            object_type_text,
            selected_object,
            static_status_text,
            selected_rigid_body -> position.x, selected_rigid_body -> position.y, selected_rigid_body -> position.z,
            selected_object_speed,
            selbt2->name
        );
    overlay_append_stats_text (information_text_buffer, sizeof (information_text_buffer));
    overlay_append_overflow_text (information_text_buffer, sizeof (information_text_buffer));
gtk_label_set_text (GTK_LABEL (debug_information_label), information_text_buffer);
}

