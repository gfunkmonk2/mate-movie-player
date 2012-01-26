/* Encoding stuff */

#ifndef IDOL_SUBTITLE_ENCODING_H
#define IDOL_SUBTITLE_ENCODING_H

#include <gtk/gtk.h>

void idol_subtitle_encoding_init (GtkComboBox *combo);
void idol_subtitle_encoding_set (GtkComboBox *combo, const char *encoding);
const char * idol_subtitle_encoding_get_selected (GtkComboBox *combo);

#endif /* SUBTITLE_ENCODING_H */
