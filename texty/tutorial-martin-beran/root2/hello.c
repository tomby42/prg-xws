/* Triviální program vyu¾ívající toolkit GTK. Zobrazí okno s tlaèítkem, po
 * kliknutí na tlaèítko skonèí. */

/* Hlavièkový soubor GTK */
#include <gtk/gtk.h>

/* Callback funkce pro událost "delete_event". Návratová hodnota FALSE zpùsobí
 * vygenerování signálu "destroy". TRUE znamená, ¾e okno nemá být zru¹eno.
 * Obecnì FALSE znamená, ¾e událost se má dále propagovat, TRUE, ¾e byla
 * obslou¾ena a nemìla by se propagovat. */
static gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    g_print("delete event occurred\n");
    return TRUE;
}

/* Callback pro signál "destroy". Funkce gtk_widget_destroy(), která po¹le
 * tento signál, zru¹í okno. My je¹tì musíme zajistit ukonèení smyèky pro
 * zpracování událostí. */
static void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

/* Callback pro "clicked" */
static void hello(GtkWidget *widget, gpointer data)
{
    g_print("Hello World\n");
}

int main(int argc, char *argv[])
{
    GtkWidget *window; /* Top-level okno */
    GtkWidget *button; /* Tlaèítko v oknì */

    /* Initializace okenního systému atoolkitu, zpracování standardních
     * pøepínaèÙ GTK:
     * --gtk-module
     * --g-fatal-warnings
     * --gtk-debug
     * --gtk-no-debug
     * --gdk-debug
     * --gdk-no-debug
     * --display
     * --sync
     * --no-xshm
     * --name
     * --class
     */
    gtk_init(&argc, &argv);

    /* Vytvoøit okno */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /* Callback funkce delete_event() se bude volat po pøijetí GTK události
     * "delete_event", typicky vygenerované window managerem. */
    g_signal_connect(G_OBJECT(window), "delete_event",
		     G_CALLBACK(delete_event), NULL);
    /* Napojení callbacku destroy() na GTK signál "destroy". Tento signál
     * je dùsledkem gtk_window_destroy() nebo vrácení FALSE z callbacku pro
     * "delete_event". */
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
    /* Nastavení ¹íøky okraje okna (oblasti, do které nebudou umís»ovány
     * vlo¾ené widgety) */
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    /* Vytvoøení tlaèítka */
    button = gtk_button_new_with_label("Hello World");
    /* Volání funkce hello() pøi kliknutí na tlaèítko */
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(hello), NULL);
    /* Pøi kliknutí na tlaèítko se zavolá gtk_widget_destroy(window). Více
     * callbackù napojených na stejný signál se volá v poøadí, v jakém byly
     * pøipojeny. */
    g_signal_connect_swapped(G_OBJECT(button), "clicked",
			      G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
    /* Vlo¾ení tlaèítka do hlavního okna */
    gtk_container_add(GTK_CONTAINER(window), button);
    /* Zobrazení tlaèítka (ve skuteènosti se objeví na obrazovce a¾ po
     * zobrazení top-level okna) */
    gtk_widget_show(button);

    /* Zobrazit hlavní okno. Obvykle se nejdøív volá gtk_widget_show() na
     * synovská okna, aby se obsah rodièovského okna vykreslil, a¾ kdy¾ je
     * uvnitø v¹e pøipraveno. */
    gtk_widget_show(window);
    
    /* Hlavní smyèka pro zpracování událostí */
    gtk_main();

    /* Konec */
    return 0;
}
