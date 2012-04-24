/* Trivi�ln� program vyu��vaj�c� toolkit GTK. Zobraz� okno s tla��tkem, po
 * kliknut� na tla��tko skon��. */

/* Hlavi�kov� soubor GTK */
#include <gtk/gtk.h>

/* Callback funkce pro ud�lost "delete_event". N�vratov� hodnota FALSE zp�sob�
 * vygenerov�n� sign�lu "destroy". TRUE znamen�, �e okno nem� b�t zru�eno.
 * Obecn� FALSE znamen�, �e ud�lost se m� d�le propagovat, TRUE, �e byla
 * obslou�ena a nem�la by se propagovat. */
static gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    g_print("delete event occurred\n");
    return TRUE;
}

/* Callback pro sign�l "destroy". Funkce gtk_widget_destroy(), kter� po�le
 * tento sign�l, zru�� okno. My je�t� mus�me zajistit ukon�en� smy�ky pro
 * zpracov�n� ud�lost�. */
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
    GtkWidget *button; /* Tla��tko v okn� */

    /* Initializace okenn�ho syst�mu atoolkitu, zpracov�n� standardn�ch
     * p�ep�na�� GTK:
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

    /* Vytvo�it okno */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /* Callback funkce delete_event() se bude volat po p�ijet� GTK ud�losti
     * "delete_event", typicky vygenerovan� window managerem. */
    g_signal_connect(G_OBJECT(window), "delete_event",
		     G_CALLBACK(delete_event), NULL);
    /* Napojen� callbacku destroy() na GTK sign�l "destroy". Tento sign�l
     * je d�sledkem gtk_window_destroy() nebo vr�cen� FALSE z callbacku pro
     * "delete_event". */
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
    /* Nastaven� ���ky okraje okna (oblasti, do kter� nebudou um�s�ov�ny
     * vlo�en� widgety) */
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    /* Vytvo�en� tla��tka */
    button = gtk_button_new_with_label("Hello World");
    /* Vol�n� funkce hello() p�i kliknut� na tla��tko */
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(hello), NULL);
    /* P�i kliknut� na tla��tko se zavol� gtk_widget_destroy(window). V�ce
     * callback� napojen�ch na stejn� sign�l se vol� v po�ad�, v jak�m byly
     * p�ipojeny. */
    g_signal_connect_swapped(G_OBJECT(button), "clicked",
			      G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));
    /* Vlo�en� tla��tka do hlavn�ho okna */
    gtk_container_add(GTK_CONTAINER(window), button);
    /* Zobrazen� tla��tka (ve skute�nosti se objev� na obrazovce a� po
     * zobrazen� top-level okna) */
    gtk_widget_show(button);

    /* Zobrazit hlavn� okno. Obvykle se nejd��v vol� gtk_widget_show() na
     * synovsk� okna, aby se obsah rodi�ovsk�ho okna vykreslil, a� kdy� je
     * uvnit� v�e p�ipraveno. */
    gtk_widget_show(window);
    
    /* Hlavn� smy�ka pro zpracov�n� ud�lost� */
    gtk_main();

    /* Konec */
    return 0;
}
