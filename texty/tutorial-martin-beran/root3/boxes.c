/* Ukázka vkládání objektù do boxù (GtkHBox, GtkVBox). Vytváøí se box
 * obsahující tlaèítka, typ a parametry boxu a parametry tlaèítek se ètou ze
 * standardního vstupu. Na ka¾dém øádku vstupu je nìkolik hodnot oddìlených
 * mezerami. První øádek definuje typ boxu (horizontální/vertikální) a
 * parametry homogeneous a spacing funkce gtk_[hv]box_new(). Ka¾dý dal¹í øádek
 * pøidá jedno tlaèítko a Po skonèení vstupního souboru se výsledný box
 * zobrazí. */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* Callback pro "delete_event" - ukonèení programu */
static gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    g_print("Received delete_event, exiting...\n");
    gtk_main_quit();
    return FALSE;
}

#define TEXT_LEN 20
#define TEXT_SLEN "20"

int main(int argc, char *argv[])
{
    GtkWidget *window, *box, *button;
    int box_type, homogeneous, spacing, border, where, expand, fill;
    unsigned int padding;
    char button_text[TEXT_LEN+1];
    
    /* Inicializace */
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//    gtk_widget_set_usize(window, 600, 400); /* Min. velikost okna */
    gtk_window_set_title(GTK_WINDOW(window), "Packing into boxes");
    g_signal_connect(G_OBJECT(window), "delete_event",
		     G_CALLBACK(delete_event), NULL);
    
    /* Vytvoøení boxu podle dat ze vstupu */
    if(isatty(STDOUT_FILENO))
	printf("box_type(0=hbox,1=vbox) homogeneous(0/1) spacing(int) "
	       "border_width(int)\n");
    if(scanf("%d %d %d %d", &box_type, &homogeneous, &spacing, &border) != 4) {
	fprintf(stderr, "Error in input\n");
	exit(-1);
    }
    if(box_type)
	box = gtk_vbox_new(homogeneous, spacing);
    else
	box = gtk_hbox_new(homogeneous, spacing);
    gtk_container_set_border_width(GTK_CONTAINER(box), border);
    gtk_container_add(GTK_CONTAINER(window), box);

    /* Vytvoøení tlaèítek podle dat ze vstupu */
    for(;;) {
	if(isatty(STDOUT_FILENO))
	    printf("where(0=start,1=end) button_text expand(0/1) fill(0/1)"
		   " padding(int)\n");
	switch( scanf("%d %" TEXT_SLEN "s %d %d %u", &where,
		      button_text, &expand, &fill, &padding) ) {
	    case EOF:
		break;
	    case 5:
		button = gtk_button_new_with_label(button_text);
		if(where)
		    gtk_box_pack_end(GTK_BOX(box), button, expand, fill,
				     padding);
		else
		    gtk_box_pack_start(GTK_BOX(box), button, expand, fill,
				       padding);
		gtk_widget_show(button);
		continue;
	    default:
		fprintf(stderr, "Error in input\n");
		exit(-1);
	}
	break;
    }
    
    /* Zobrazení okna a hlavní smyèka */
    gtk_widget_show(box);
    gtk_widget_show(window);

    gtk_main();

    return 0;
}
