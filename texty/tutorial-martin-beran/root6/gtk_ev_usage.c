/* Pøíklad vytvoøení nového widgetu (upravený widget GtkEv z knihy
 * Havoc Pennington: GTK+/Gnome Application Development).
 * Toto je program, který pou¾ívá widget GtkEv. */

#include "gtkev.h"
#include <gtk/gtk.h>
#include <string.h>

/* Handler pro signál "event_received" */
static void ev_recv(GtkWidget *widget, GdkEventType ev_type, gpointer data)
{
    GValue val;

    bzero(&val, sizeof(val));
    g_value_init(&val, G_TYPE_UINT);
    g_object_get_property(G_OBJECT(widget), "ev_count", &val);
    g_print("%s (type=%u, count=%u)\n", (char *)data, (guint)ev_type, 
            g_value_get_uint(&val));
}

/* Vynulování èítaèe událostí */
static void reset_counter(GtkWidget *widget, gpointer data)
{
    gtk_object_set(GTK_OBJECT(data), "ev_count", (guint) 0, NULL);
}

int main(int argc, char *argv[])
{
    GtkWidget *window, *box, *ev_w, *widget;

    gtk_set_locale();
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "destroy",
		     G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_title(GTK_WINDOW(window), "Creating a widget");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    box = gtk_vbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    ev_w = widget = gtk_ev_new();
    g_signal_connect(G_OBJECT(widget), "event_received",
		     G_CALLBACK(ev_recv), (char *) "Event received");
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    
    /* Pøidáme do okna je¹tì tlaèítko, aby bylo dobøe vidìt pøepínání focusu */
    widget = gtk_button_new_with_label("Reset counter");
    g_signal_connect(G_OBJECT(widget), "clicked",
		     G_CALLBACK(reset_counter), ev_w);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
