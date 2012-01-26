
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#define IDOL_OBJECT_HAS_SIGNAL(obj, name) (g_signal_lookup (name, g_type_from_name (G_OBJECT_TYPE_NAME (obj))) != 0)

void idol_gdk_window_set_invisible_cursor (GdkWindow *window);
void idol_gdk_window_set_waiting_cursor (GdkWindow *window);

gboolean idol_display_is_local (void);

char *idol_time_to_string (gint64 msecs);
gint64 idol_string_to_time (const char *time_string);
char *idol_time_to_string_text (gint64 msecs);

void idol_widget_set_preferred_size (GtkWidget *widget, gint width,
				      gint height);
gboolean idol_ratio_fits_screen (GdkWindow *window, int video_width,
				  int video_height, gfloat ratio);
