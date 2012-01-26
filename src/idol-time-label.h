
#ifndef IDOL_TIME_LABEL_H
#define IDOL_TIME_LABEL_H

#include <gtk/gtk.h>

#define IDOL_TYPE_TIME_LABEL            (idol_time_label_get_type ())
#define IDOL_TIME_LABEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_TIME_LABEL, IdolTimeLabel))
#define IDOL_TIME_LABEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_TIME_LABEL, IdolTimeLabelClass))
#define IDOL_IS_TIME_LABEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_TIME_LABEL))
#define IDOL_IS_TIME_LABEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_TIME_LABEL))

typedef struct IdolTimeLabel	      IdolTimeLabel;
typedef struct IdolTimeLabelClass    IdolTimeLabelClass;
typedef struct _IdolTimeLabelPrivate IdolTimeLabelPrivate;

struct IdolTimeLabel {
	GtkLabel parent;
	IdolTimeLabelPrivate *priv;
};

struct IdolTimeLabelClass {
	GtkLabelClass parent_class;
};

G_MODULE_EXPORT GType idol_time_label_get_type (void);
GtkWidget *idol_time_label_new                 (void);
void       idol_time_label_set_time            (IdolTimeLabel *label,
                                                 gint64 time, gint64 length);
void       idol_time_label_set_seeking         (IdolTimeLabel *label,
                                                 gboolean seeking);

#endif /* IDOL_TIME_LABEL_H */
