/* P��klad vytvo�en� nov�ho widgetu (upraven� widget GtkEv z knihy
 * Havoc Pennington: GTK+/Gnome Application Development).
 * Tento hlavi�kov� soubor definuje interface widgetu. */

#ifndef GTKEV_H
#define GTKEV_H

#include <gtk/gtk.h>

/* GtkEv je widget, kter� napodobuje program xev: m� 2 GDK (a tedy X) okna, v
 * jednom chyt� ud�losti, ve druh�m je zobrazuje. */

/* Makra pro zji�t�n� typu a p�etypov�n� */
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
    GtkWidget widget; /* Rodi�ovsk� struktura mus� b�t prvn� */
    
    GdkWindow *ev_win; /* Okno pro ud�losti (vnit�n�), vn�j�� okno je 
                          widget.window */
    GdkRectangle ev_win_rect; /* Oblast p�ekryt� vnit�n�m oknem */
    GdkRectangle list_rect; /* Oblast p�ekryt� seznamem ud�lost� */
    GList *list; /* Seznam zachycen�ch ud�lost� */
    GList *list_end; /* Konec seznamu */
    gint list_len; /* Po�et ud�lost� v seznamu */
    guint ev_count; /* Celkov� po�et zachycen�ch ud�lost� */
    gboolean constructed; /* Prob�hl konstruktor pro objekt */
};

/* Ve�ejn� funkce */
GtkType gtk_ev_get_type(void); /* Identifik�tor typu */
GtkWidget *gtk_ev_new(void); /* Vytvo�en� objektu */

/* Struktura pro t��du */
struct _GtkEvClass {
    GtkWidgetClass parent_class; /* Rodi�ovsk� struktura mus� b�t prvn� */

    void (*event_received)(GtkWidget *widget, GdkEventType ev_type,
                           gpointer data); /* Defaultn� handler sign�lu
                                              "event_received" */
};

#endif /* GTKEV_H */
