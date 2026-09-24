#ifndef overlay_h
#define overlay_h

#include <gtk/gtk.h>
#include "../core/math3D.h"

GtkWidget *overlay_initialise (GtkWidget *gl_drawing_area_widget);
void overlay_update (void);

void overlay_show_hotbar (bool show);
void overlay_show_inventory (bool show);
void overlay_set_selected_slot (int slot);
void overlay_update_health_hunger (float health, float max_health, float hunger, float max_hunger);
void overlay_set_game_mode_text (const char* text);

#endif