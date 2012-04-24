/* Pøíklad vytvoøení nového widgetu (upravený widget GtkEv z knihy
 * Havoc Pennington: GTK+/Gnome Application Development).
 * Tento hlavièkový soubor definuje interface widgetu. */

#ifndef GTKEV_H
#define GTKEV_H

#include <gtk/gtk.h>

/* GtkEv je widget, který napodobuje program xev: má 2 GDK (a tedy X) okna, v
 * jednom chytá události, ve druhém je zobrazuje. */

/* Makra pro zji¹tìní typu a pøetypování */
#define GTK_TYPE_EV (gtk_ev_get_type())
#define GTK_EV(obj) (GTK_CHECK_CAST((obj), GTK_TYPE_EV, GtkEv))
#define GTK_EV_CLASS(klass) \
    (GTK_CHECK_CLASS_CAST((klass), GTK_TYPE_EV, GtkEvClass))
#define GTK_IS_EV(obj) (GTK_CHECK_TYPE((obj), GTK_TYPE_EV))
#define GTK_IS_EV_CLASS(klass) (GTK_CHECK_CLASS_TYPE((klass), GTK_TYPE_EV))

typedef struct _GtkEvClass GtkEvClass;
typedef struct _GtkEv GtkEv;

/* Struktura pro widget */
struct _GtkEv {
    GtkWidget widget; /* Rodièovská struktura musí být první */
    
    GdkWindow *ev_win; /* Okno pro události (vnitøní), vnìj¹í okno je 
                          widget.window */
    GdkRectangle ev_win_rect; /* Oblast pøekrytá vnitøním oknem */
    GdkRectangle list_rect; /* Oblast pøekrytá seznamem událostí */
    GList *list; /* Seznam zachycených událostí */
    GList *list_end; /* Konec seznamu */
    gint list_len; /* Poèet událostí v seznamu */
    guint ev_count; /* Celkový poèet zachycených událostí */
    gboolean constructed; /* Probìhl konstruktor pro objekt */
};

/* Veøejné funkce */
GtkType gtk_ev_get_type(void); /* Identifikátor typu */
GtkWidget *gtk_ev_new(void); /* Vytvoøení objektu */

/* Struktura pro tøídu */
struct _GtkEvClass {
    GtkWidgetClass parent_class; /* Rodièovská struktura musí být první */

    void (*event_received)(GtkWidget *widget, GdkEventType ev_type,
                           gpointer data); /* Defaultní handler signálu
                                              "event_received" */
};

#endif /* GTKEV_H */
