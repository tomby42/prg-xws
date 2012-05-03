#include <gtk/gtk.h>


static void hello( GtkWidget *widget,
                   gpointer   data )
{
    g_print ("Nazdar svete\n");
}

int main( int   argc,
          char *argv[] )
{
    GtkWidget *window;
    GtkWidget *button;
    
    gtk_init (&argc, &argv);
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    
    button = gtk_button_new_with_label ("Nazdar svete");
    
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (hello), NULL);
        
    gtk_container_add (GTK_CONTAINER (window), button);
    
    gtk_widget_show (button);
    
    gtk_widget_show (window);
    
    gtk_main ();
    
    return 0;
}
