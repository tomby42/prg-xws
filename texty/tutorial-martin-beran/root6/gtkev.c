/* P��klad vytvo�en� nov�ho widgetu (upraven� widget GtkEv z knihy
 * Havoc Pennington: GTK+/Gnome Application Development).
 * Tento hlavi�kov� soubor definuje implementaci widgetu. */

#include "gtkev.h"

/* Typy sign�l� definovan�ch widgetem */
enum {
    EVENT_RECEIVED,
    LAST_SIGNAL, /* nen� sign�l, definuje velikost ev_signals[] */
};

/* Properties definovan� widgetem */
enum {
    PROP_0, /* neplatn� id property */
    PROP_EV_COUNT,
};

/* Tabulka sign�l� */
static guint ev_signals[LAST_SIGNAL];

/* Odkaz na rodi�ovskou t��du pou��van� pro vol�n� metod p�edka */
static GtkWidgetClass *parent_class = NULL;

/* Deklarace statick�ch funkc�, kter� jsou definov�ny d�le v tomto souboru */
static void gtk_ev_class_init(GtkEvClass *klass);
static void gtk_ev_init(GtkEv *ev, GtkEvClass *ev_class);
static GObject *gtk_ev_constructor(GType type, guint n_const_prop,
				   GObjectConstructParam *const_params);
static void gtk_ev_destroy(GtkObject *object);
static void gtk_ev_set_property(GObject *object, guint prop_id,
				const GValue *value, GParamSpec *spec);
static void gtk_ev_get_property(GObject *object, guint prop_id, GValue *value,
	     			GParamSpec *spec);
static void gtk_ev_realize(GtkWidget *widget);
static void gtk_ev_unrealize(GtkWidget *widget);
static void gtk_ev_size_request(GtkWidget *widget, GtkRequisition *req);
static void gtk_ev_size_allocate(GtkWidget *widget, GtkAllocation *alloc);
static gint gtk_ev_expose(GtkWidget *widget, GdkEventExpose *event);
static void gtk_ev_paint(GtkEv *ev, GdkRectangle *area);
static void gtk_ev_paint_ev_win(GtkEv *ev, GdkRectangle *area);
static gint gtk_ev_event(GtkWidget *widget, GdkEvent *event);
static void gtk_ev_set_arg(GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_ev_get_arg(GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_real_event_received(GtkWidget *widget, GdkEventType ev_type,
                                    gpointer data);
static gchar* event_to_text(GdkEvent* event);
static void gtk_ev_push_text(GtkEv *ev, const gchar* text);
static gchar* event_name_line(GdkEvent* event);
static gchar* any_event_line(GdkEvent* event);
static gchar* event_state_line(GdkModifierType state);
static gint gtk_ev_button(GtkWidget *widget, GdkEventButton *event);

/* Inicializace typu GtkEv */
GType gtk_ev_get_type(void)
{
    static GType ev_type = G_TYPE_INVALID; /* 0 */

    if(ev_type == G_TYPE_INVALID) {
        static const GTypeInfo ev_info = {
            sizeof(GtkEvClass), /* velikost t��dy */
	    NULL, /* inicializace b�zov� t��dy, pou�ila by
		     se, pokud by nesta�ilo, �e se v GtkEvClass.parent_class
		     vyrob� kopie GtkWidget class */
	    NULL, /* finalizace b�zov� t��dy */
            (GClassInitFunc)gtk_ev_class_init, /* inicializace t��dy */
	    NULL, /* finalizace t��dy */
	    NULL, /* data pro inicializaci/finalizaci t��dy */
            sizeof(GtkEv), /* velikost objektu */
	    0, /* po�et prealokovan�ch instanc� */
            (GInstanceInitFunc)gtk_ev_init, /* inicializace objektu */
	    NULL /* funkce pro GValue tohoto typu */
        };
	ev_type = g_type_register_static(GTK_TYPE_WIDGET, "GtkEv", &ev_info, 0);
    }
    return ev_type;
}

/* Inicializace t��dy GtkEvClass */
static void gtk_ev_class_init(GtkEvClass *klass)
{
    GObjectClass *g_object_class = G_OBJECT_CLASS(klass);
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    g_object_class->constructor = gtk_ev_constructor;
    g_object_class->set_property = gtk_ev_set_property;
    g_object_class->get_property = gtk_ev_get_property;

    object_class->destroy = gtk_ev_destroy;

    widget_class->realize = gtk_ev_realize;
    widget_class->unrealize = gtk_ev_unrealize;
    widget_class->size_request = gtk_ev_size_request;
    widget_class->size_allocate = gtk_ev_size_allocate;
    widget_class->expose_event = gtk_ev_expose;
    widget_class->event = gtk_ev_event;
    widget_class->button_press_event = gtk_ev_button;

    /* Registrace properties */
    g_object_class_install_property(g_object_class, PROP_EV_COUNT,
	g_param_spec_uint("ev_count", "Ev count", "Event count", 0, G_MAXUINT,
			  0, G_PARAM_READABLE | G_PARAM_WRITABLE));

    /* Registrace nov�ch sign�l� */
    ev_signals[EVENT_RECEIVED] =
	g_signal_new("event_received", GTK_TYPE_EV, G_SIGNAL_RUN_FIRST,
		     G_STRUCT_OFFSET(GtkEvClass, event_received), NULL, NULL,
		     gtk_marshal_VOID__ENUM, G_TYPE_NONE, 1, G_TYPE_INT);
    klass->event_received = gtk_real_event_received;
}

/* Inicializace objektu GtkEv */
static void gtk_ev_init(GtkEv *ev, GtkEvClass *ev_class)
{
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(ev), GTK_CAN_FOCUS);
    ev->ev_win_rect.x = ev->ev_win_rect.y = 0;
    ev->ev_win_rect.width = ev->ev_win_rect.height = 0;
    ev->list_rect.x = ev->list_rect.y = 0;
    ev->list_rect.width = ev->list_rect.height = 0;
    ev->list = ev->list_end = NULL;
    ev->list_len = 0;
    ev->ev_count = 0;
    ev->constructed = FALSE;
}

/* Vytvo�en� nov�ho objektu GtkEv */
GtkWidget *gtk_ev_new(void)
{
    return GTK_WIDGET(gtk_type_new(GTK_TYPE_EV));
}

/* Konstrukce objektu */
static GObject *gtk_ev_constructor(GType type, guint n_const_prop,
				   GObjectConstructParam *const_params)
{
    GObject *object;
    GtkEv *ev;

    object = G_OBJECT_CLASS(parent_class)->constructor(type, n_const_prop,
						      const_params);
    ev = GTK_EV(object);
    ev->constructed = TRUE; /* Tohle je tu jen na uk�zku, proto�e jinak nem�me
			       do konstruktoru co d�t a ani bychom ho nemuseli
			       definovat. */
    
    return object;
}

/* Destrukce objektu */
static void gtk_ev_destroy(GtkObject *object)
{
    GtkEv *ev;
    GList *p;

    /* Test platnosti parametru */
    g_return_if_fail(GTK_IS_EV(object));

    ev = GTK_EV(object);

    for(p = ev->list; p; p = p->next)
        g_strfreev((gchar **) p->data);
    g_list_free(ev->list);

    ev->list = ev->list_end = NULL;
    ev->list_len = 0;

    /* Vol�n� metody p�edka (chaining) */
    if(GTK_OBJECT_CLASS(parent_class)->destroy)
        GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

static void gtk_ev_set_property(GObject *object, guint prop_id,
				const GValue *value, GParamSpec *spec)
{
    GtkEv *ev;
    
    g_return_if_fail(GTK_IS_EV(object));
    ev = GTK_EV(object);

    switch(prop_id) {
    case PROP_EV_COUNT:
	ev->ev_count = g_value_get_uint(value);
	break;
    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, spec);
	break;
    }
}

static void gtk_ev_get_property(GObject *object, guint prop_id, GValue *value,
	     			GParamSpec *spec)
{
    GtkEv *ev;

    g_return_if_fail(GTK_IS_EV(object));
    ev = GTK_EV(object);

    switch(prop_id) {
    case PROP_EV_COUNT:
	g_value_set_uint(value, ev->ev_count);
	break;
    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, spec);
	break;
    }
}

/* Vytvo�en� GDK/X oken pro widget */
static void gtk_ev_realize(GtkWidget *widget)
{
    /* Default v GtkWidget je vhodn� jen pro widgety bez GDK/X oken */
    GdkWindowAttr attr;
    gint attr_mask;
    GtkEv *ev;
    GdkCursor *cursor;

    g_return_if_fail(GTK_IS_EV(widget));

    ev = GTK_EV(widget);

    /* Nastavit p��znak realizace */
    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

    /* Hlavn� okno widgetu (GtkEv.window) */
    attr.window_type = GDK_WINDOW_CHILD;
    attr.x = widget->allocation.x;
    attr.y = widget->allocation.y;
    attr.width = widget->allocation.width;
    attr.height = widget->allocation.height;
    attr.wclass = GDK_INPUT_OUTPUT;
    attr.visual = gtk_widget_get_visual(widget);
    attr.colormap = gtk_widget_get_colormap(widget);
    attr.event_mask = gtk_widget_get_events(widget) |
        GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK;

    attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
                                    &attr, attr_mask);
    gdk_window_set_user_data(widget->window, widget); /* Odkaz z GDK okna na
                                                         p��slu�n� widget */

    /* Vnit�n� okno pro ud�losti (GtkEv.ev_window) */
    attr.window_type = GDK_WINDOW_CHILD;
    attr.x = ev->ev_win_rect.x;
    attr.y = ev->ev_win_rect.y;
    attr.width = ev->ev_win_rect.width;
    attr.height = ev->ev_win_rect.height;
    attr.wclass = GDK_INPUT_OUTPUT;
    attr.visual = gtk_widget_get_visual(widget);
    attr.colormap = gtk_widget_get_colormap(widget);
    attr.event_mask = GDK_ALL_EVENTS_MASK;
    attr.cursor = cursor = gdk_cursor_new(GDK_CROSSHAIR);

    attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP |
        GDK_WA_CURSOR;
    
    ev->ev_win = gdk_window_new(widget->window, &attr, attr_mask);
    gdk_window_set_user_data(ev->ev_win, widget);

    gdk_window_show(ev->ev_win); /* gtk_widget_show() zobraz� pouze 
                                        GtkEv.window */

    gdk_cursor_destroy(cursor); /* Odkaz na kurzor u� d�l nepot�ebujeme, kurzor
                                   bude zru�en, a� na n�j nebude odkazovat
                                   GtkEv.ev_win */
    
    /* P�id�n� stylu k widgetu. Mus� se prov�st po vytvo�en� GDK oken, proto�e
     * styl obsahuje odkazy na prost�edky (resources) X */
    widget->style = gtk_style_attach(widget->style, widget->window);
    gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
    gdk_window_set_background(ev->ev_win,
                              &widget->style->base[GTK_STATE_NORMAL]);

    /* Nevolat metodu p�edka, proto�e nen� pou�iteln� pro GtkEv */
}

/* Zru�en� GDK/X oken widgetu */
static void gtk_ev_unrealize(GtkWidget *widget)
{
    GtkEv *ev;

    g_return_if_fail(GTK_IS_EV(widget));

    ev = GTK_EV(widget);

    /* Schovat okna */
    if(GTK_WIDGET_MAPPED(widget))
        gtk_widget_unmap(widget);

    GTK_WIDGET_UNSET_FLAGS(widget, GTK_MAPPED);

    /* Zru�it vnit�n� okno (GtkEv.ev_win) */
    if(ev->ev_win) {
        gdk_window_set_user_data(ev->ev_win, NULL);
        gdk_window_destroy(ev->ev_win);
        ev->ev_win = NULL;
    }

    /* Zru�it hlavn� okno widgetu (GtkEv.window), zru�it p��znak realizace */
    if(GTK_WIDGET_CLASS(parent_class))
       GTK_WIDGET_CLASS(parent_class)->unrealize(widget);
}

/* Po�adavek widgetu na svoj� velikost */
static void gtk_ev_size_request(GtkWidget *widget, GtkRequisition *req)
{
    /* Defaultn� metoda size request widgetu GtkWidget vr�t� aktu�ln�
     * widget->requisition. Proto by GtkEv mohlo inicializovat
     * widget->requisition v gtk_ev_init() a m�sto definice vlastn�ho
     * gtk_ev_size_requst() bychom mohli pou��t zd�d�nou metodu */
    g_return_if_fail(GTK_IS_EV(widget));
    g_return_if_fail(req);

    /* GtkEv v�dy po�aduje fixn� velikost */
    req->width = 400;
    req->height = 300;
}

/* P�id�len� velikosti widgetu */
static void gtk_ev_size_allocate(GtkWidget *widget, GtkAllocation *alloc)
{
    static const gint spacing = 10;
    GtkEv *ev;

    g_return_if_fail(GTK_IS_EV(widget));
    g_return_if_fail(alloc);

    ev = GTK_EV(widget);

    widget->allocation = *alloc;

    ev->ev_win_rect.width = MAX(alloc->width-2*spacing, 0);
    ev->ev_win_rect.height = MAX(alloc->height/5-2*spacing, 0);
    ev->ev_win_rect.x = (alloc->width - ev->ev_win_rect.width)/2;
    ev->ev_win_rect.y = MIN(spacing, alloc->height);

    ev->list_rect.x = ev->ev_win_rect.x;
    ev->list_rect.y = ev->ev_win_rect.y + ev->ev_win_rect.height + spacing;
    ev->list_rect.width = ev->ev_win_rect.width;
    ev->list_rect.height = 
        MAX(alloc->height - ev->ev_win_rect.height - 3*spacing, 0);

    if(GTK_WIDGET_REALIZED(widget)) {
        gdk_window_move_resize(widget->window, alloc->x, alloc->y,
                               alloc->width, alloc->height);
        gdk_window_move_resize(ev->ev_win, ev->ev_win_rect.x,
                               ev->ev_win_rect.y, ev->ev_win_rect.width,
                               ev->ev_win_rect.height);
    }
}

/* P�ekreslen� okna jako reakce na GdkEventExpose */
static gint gtk_ev_expose(GtkWidget *widget, GdkEventExpose *event)
{
    g_return_val_if_fail(GTK_IS_EV(widget), FALSE);
    
    if(event->window == widget->window)
        gtk_ev_paint(GTK_EV(widget), &event->area);
    else
        if(event->window == GTK_EV(widget)->ev_win)
            gtk_ev_paint_ev_win(GTK_EV(widget), &event->area);

    return TRUE;
}

/* Kreslic� funkce spole�n� pro gtk_ev_draw() a gtk_ev_expose(), kter� kresl�
 * hlavn� okno widgetu */
static void gtk_ev_paint(GtkEv *ev, GdkRectangle *area)
{
    GtkWidget *widget;
    
    g_return_if_fail(ev);
    g_return_if_fail(area);

    widget = GTK_WIDGET(ev);

    if(!GTK_WIDGET_DRAWABLE(widget))
        return; /* Not visible and mapped */

    gdk_window_clear_area(widget->window, area->x, area->y, area->width,
                          area->height);
    gdk_gc_set_clip_rectangle(widget->style->black_gc, area);

    /* �ern� r�me�ek kolem vnit�n�ho okna */
    gdk_draw_rectangle(widget->window, widget->style->black_gc, FALSE,
                       ev->ev_win_rect.x-1, ev->ev_win_rect.y-1,
                       ev->ev_win_rect.width+2, ev->ev_win_rect.height+2);

    gdk_gc_set_clip_rectangle(widget->style->black_gc, NULL);

    /* Text (seznam zachycen�ch ud�lost�) */
    if(ev->list) {
        GdkRectangle intersect;

        if(gdk_rectangle_intersect(&ev->list_rect, area, &intersect)) {
            static const gint space = 2;
            gint line, step, first_baseline;
            GList *p;
	    GdkFont *font = gtk_style_get_font(widget->style);

            step = font->ascent + font->descent + space;
            first_baseline = ev->list_rect.y + font->ascent + space;
            line = 0;
            for(p = ev->list; p; p = g_list_next(p)) {
                gchar **pev = p->data;
                gint i;
                
                for(i = 0; pev[i]; i++) {
                    gtk_paint_string(widget->style, widget->window,
                                     widget->state, &intersect, widget,
                                     (char *) "ev", ev->list_rect.x,
                                     first_baseline + line*step, pev[i]);
                    line++;
                }
                
                if(first_baseline + line*step - 2*step >
                   intersect.y + intersect.height)
                    break;
            }
        }
    }

    /* Grafick� zv�razn�n�, kdy� m� okno focus */
    if(GTK_WIDGET_HAS_FOCUS(widget))
        gtk_paint_focus(widget->style, widget->window, GTK_WIDGET_STATE(widget),
			area, widget, (char *) "ev", 0, 0, -1, -1);
}

/* Kreslic� funkce spole�n� pro gtk_ev_draw() a gtk_ev_expose(), kter� kresl�
 * vnit�n� okno widgetu */
static void gtk_ev_paint_ev_win(GtkEv *ev, GdkRectangle *area)
{
    GtkWidget *widget;
    gint width, x, y;
    const char *title = "Event Window";
    GdkFont *font;

    g_return_if_fail(ev);
    g_return_if_fail(area);

    widget = GTK_WIDGET(ev);
    font = gtk_style_get_font(widget->style);
    if(!GTK_WIDGET_DRAWABLE(ev))
        return;

    gdk_window_clear_area(ev->ev_win, area->x, area->y,
                          area->width, area->height);

    gdk_gc_set_clip_rectangle(widget->style->black_gc, area);

    width = gdk_string_width(font, title);

    x = (ev->ev_win_rect.width - width)/2;
    y = font->ascent + 2;

    gdk_draw_string(ev->ev_win, font, widget->style->black_gc, x, y, title);

    gdk_gc_set_clip_rectangle(widget->style->black_gc, NULL);
}

/*
 * Stisk tla��tka my�i - z�skat focus
 */
static gint gtk_ev_button(GtkWidget *widget, GdkEventButton *event)
{
    g_return_val_if_fail(widget, FALSE);
    g_return_val_if_fail(event, FALSE);

    if(event->type == GDK_BUTTON_PRESS)
        gtk_widget_grab_focus(widget);

    return FALSE;
}

/* Zpracov�n� ud�lost� - tato funkce p�id�v� popis ud�losti do seznamu
 * zachycen�ch ud�lost�, ale vrac� FALSE, proto�e chceme, aby se ud�losti d�l
 * norm�ln� zpracov�valy. */
static gint gtk_ev_event(GtkWidget *widget, GdkEvent *event)
{
    GtkEv *ev;

    g_return_val_if_fail(widget, FALSE);
    g_return_val_if_fail(GTK_IS_EV(widget), FALSE);
    g_return_val_if_fail(event, FALSE);

    ev = GTK_EV(widget);

    if(event->any.window == widget->window) {
        if(GTK_WIDGET_CLASS(parent_class)->event)
            return GTK_WIDGET_CLASS(parent_class)->event(widget, event);
    } else {
        /* Ud�lost je bu� ur�ena pro ev->ev_win, nebo je to kl�vesnicov�
         * ud�lost poslan� z toplevel GtkWindow */
        gchar *text;

        text = event_to_text(event);
        gtk_ev_push_text(ev, text);
        g_free(text);

        /* Zajistit p�e�ten� dal�� motion event */
        if(event->type == GDK_MOTION_NOTIFY)
            gdk_window_get_pointer(ev->ev_win, NULL, NULL, NULL);
    }
    
    gtk_signal_emit(GTK_OBJECT(widget), ev_signals[EVENT_RECEIVED],
                    event->type);

    return FALSE;
}

/* Defaultn� handler sign�lu "event_received" */
static void gtk_real_event_received(GtkWidget *widget, GdkEventType ev_type,
                                    gpointer data)
{
    g_return_if_fail(widget);
    g_return_if_fail(GTK_IS_EV(widget));

    GTK_EV(widget)->ev_count++;
}

/* Vytvo�en� textov�ho popisu ud�losti */
static gchar* event_to_text(GdkEvent* event)
{
    gchar* any_line = any_event_line(event);
    gchar* name_line = event_name_line(event);
    gchar* entire_line = NULL, *detail = NULL, *state = NULL;

    switch(event->type) {
        case GDK_NOTHING:
            break;
        case GDK_DELETE:
            break;
        case GDK_DESTROY:
            break;
        case GDK_EXPOSE:
            detail = g_strdup_printf("Area: %d,%d  %dx%d Count: %d\n",
                                     event->expose.area.x, 
                                     event->expose.area.y,
                                     event->expose.area.width,
                                     event->expose.area.height,
                                     event->expose.count);
            break;
        case GDK_MOTION_NOTIFY:
            detail = g_strdup_printf("x: %g y: %g\n",
                                     event->motion.x, event->motion.y);
            state = event_state_line(event->motion.state);
            break;
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            detail = g_strdup_printf("Button: %d\n", event->button.button);
            state = event_state_line(event->button.state);
            break;
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
            detail = g_strdup_printf("Keyval: GDK_%s Text: %s\n",
                                     gdk_keyval_name(event->key.keyval),
                                     event->key.string);
            state = event_state_line(event->key.state);
            break;
        case GDK_ENTER_NOTIFY:
            break;
        case GDK_LEAVE_NOTIFY:
            break;
        case GDK_FOCUS_CHANGE:
            break;
        case GDK_CONFIGURE:
            break;
        case GDK_MAP:
            break;
        case GDK_UNMAP:
            break;
        case GDK_PROPERTY_NOTIFY:
            break;
        case GDK_SELECTION_CLEAR:
            break;
        case GDK_SELECTION_REQUEST:
            break;
        case GDK_SELECTION_NOTIFY:
            break;
        case GDK_PROXIMITY_IN:
            break;
        case GDK_PROXIMITY_OUT:
            break;
        case GDK_DRAG_ENTER:
            break;
        case GDK_DRAG_LEAVE:
            break;
        case GDK_DRAG_MOTION:
            break;
        case GDK_DRAG_STATUS:
            break;
        case GDK_DROP_START:
            break;
        case GDK_DROP_FINISHED:
            break;
        case GDK_CLIENT_EVENT:
            break;
        case GDK_VISIBILITY_NOTIFY:
            break;
        case GDK_NO_EXPOSE:
            break;
        default:
            g_assert_not_reached();
            break;
    }

    if(entire_line == NULL)
        entire_line = g_strconcat(name_line, "  ", any_line, 
                                  detail ? "  " : NULL, detail, 
                                  state ? "  " : NULL, state,
                                  NULL);

    g_free(name_line);
    g_free(any_line);
    g_free(detail);
    g_free(state);
    
    return entire_line;
}

/* P�id�n� textov�ho popisu ud�losti do seznamu zachycen�ch ud�lost� we
 * widgetu */
static void gtk_ev_push_text(GtkEv *ev, const gchar* text)
{  
    g_return_if_fail(ev);
    
    if(text) {
        gchar** event; 
        
        event = g_strsplit(text, "\n", 10);
        ev->list = g_list_prepend(ev->list, event);
        ev->list_len ++;
        
        if(ev->list_end == NULL)
            ev->list_end = ev->list;
       
       if(ev->list_len > 100) {
           GList* prev = ev->list_end->prev;
           
           prev->next = NULL;
           g_strfreev(ev->list_end->data);
           g_list_free_1(ev->list_end);
           ev->list_end = prev;
           ev->list_len--;
         }
     }

    if(GTK_WIDGET_DRAWABLE(ev))
        gtk_widget_queue_draw_area(GTK_WIDGET(ev), ev->list_rect.x,
                                   ev->list_rect.y, ev->list_rect.width,
                                   ev->list_rect.height);
}

/* P�evod typu ud�losti na �et�zec */
static gchar* event_name_line(GdkEvent* event)
{
    switch(event->type)
    {
        case GDK_NOTHING:
            return g_strdup("Invalid event!\n");
            break;
        case GDK_DELETE:
            return g_strdup("Delete\n");
            break;
        case GDK_DESTROY:
            return g_strdup("Destroy\n");
            break;
        case GDK_EXPOSE:
            return g_strdup("Expose\n");
            break;
        case GDK_MOTION_NOTIFY:
            return g_strdup("Motion Notify\n");
            break;
        case GDK_BUTTON_PRESS:
            return g_strdup("Button Press\n");
            break;
        case GDK_2BUTTON_PRESS:
            return g_strdup("2 Button Press\n");
            break;
        case GDK_3BUTTON_PRESS:
            return g_strdup("3 Button Press\n");
            break;
        case GDK_BUTTON_RELEASE:
            return g_strdup("Button Release\n");
            break;
        case GDK_KEY_PRESS:
            return g_strdup("Key Press\n");
            break;
        case GDK_KEY_RELEASE:
            return g_strdup("Key Release\n");
            break;
        case GDK_ENTER_NOTIFY:
            return g_strdup("Enter Notify\n");
            break;
        case GDK_LEAVE_NOTIFY:
            return g_strdup("Leave Notify\n");
            break;
        case GDK_FOCUS_CHANGE:
            return g_strdup("Focus Change\n");
            break;
        case GDK_CONFIGURE:
            return g_strdup("Configure\n");
            break;
        case GDK_MAP:
            return g_strdup("Map\n");
            break;
        case GDK_UNMAP:
            return g_strdup("Unmap\n");
            break;
        case GDK_PROPERTY_NOTIFY:
            return g_strdup("Property Notify\n");
            break;
        case GDK_SELECTION_CLEAR:
            return g_strdup("Selection Clear\n");
            break;
        case GDK_SELECTION_REQUEST:
            return g_strdup("Selection Request\n");
            break;
        case GDK_SELECTION_NOTIFY:
            return g_strdup("Selection Notify\n");
            break;
        case GDK_PROXIMITY_IN:
            return g_strdup("Proximity In\n");
            break;
        case GDK_PROXIMITY_OUT:
            return g_strdup("Proximity Out\n");
            break;
        case GDK_DRAG_ENTER:
            return g_strdup("Drag Enter\n");
            break;
        case GDK_DRAG_LEAVE:
            return g_strdup("Drag Leave\n");
            break;
        case GDK_DRAG_MOTION:
            return g_strdup("Drag Motion\n");
            break;
        case GDK_DRAG_STATUS:
            return g_strdup("Drag Status\n");
            break;
        case GDK_DROP_START:
            return g_strdup("Drop Start\n");
            break;
        case GDK_DROP_FINISHED: 
            return g_strdup("Drop Finished\n");
            break;
        case GDK_CLIENT_EVENT:
            return g_strdup("Client Event\n");
            break;
        case GDK_VISIBILITY_NOTIFY:
            return g_strdup("Visibility Notify\n");
            break;
        case GDK_NO_EXPOSE:
            return g_strdup("No Expose\n");
            break;
        default:
            g_assert_not_reached();
            return NULL;
            break;
    }
}

/* �et�zcov� z�pis informac� spole�n�ch pro v�echny typy ud�lost� */
static gchar* any_event_line(GdkEvent* event)
{
    guint32 event_time;

    event_time = gdk_event_get_time(event);
    
    if(event_time != GDK_CURRENT_TIME)
        return g_strdup_printf("Window: %p Time: %u send_event: %s\n",
                               event->any.window, event_time,
                               event->any.send_event ? "True" : "False");
    else
        return g_strdup_printf("Window: %p send_event: %s\n",
                               event->any.window, 
                               event->any.send_event ? "True" : "False");
}

#define MAX_STATES 30

/* Dal�� informace pro ud�losti kl�vesnice a my�i */
static gchar* event_state_line(GdkModifierType state)
{
    gchar** states;
    gint n_active;
    
    states = g_new(gchar*, MAX_STATES);
    
    n_active = 0;
    
    if(state & GDK_SHIFT_MASK)
        states[n_active++] = (char *) "Shift";
    if(state & GDK_LOCK_MASK)
        states[n_active++] = (char *) "Lock";
    if(state & GDK_CONTROL_MASK)
        states[n_active++] = (char *) "Ctrl";
    if(state & GDK_MOD1_MASK)
        states[n_active++] = (char *) "Mod1";
    if(state & GDK_MOD2_MASK)
        states[n_active++] = (char *) "Mod2";
    if(state & GDK_MOD3_MASK)
        states[n_active++] = (char *) "Mod3";
    if(state & GDK_MOD4_MASK)
        states[n_active++] = (char *) "Mod4";
    if(state & GDK_MOD5_MASK)
        states[n_active++] = (char *) "Mod5";
    if(state & GDK_BUTTON1_MASK)
        states[n_active++] = (char *) "Button1";
    if(state & GDK_BUTTON2_MASK)
        states[n_active++] = (char *) "Button2";
    if(state & GDK_BUTTON3_MASK)
        states[n_active++] = (char *) "Button3";
    if(state & GDK_BUTTON4_MASK)
        states[n_active++] = (char *) "Button4";
    if(state & GDK_BUTTON5_MASK)
        states[n_active++] = (char *) "Button4";
    if(state & GDK_RELEASE_MASK)
        states[n_active++] = (char *) "Release";

    if (n_active == 0)
        return NULL;
    else {
        gchar* str = NULL;
        gchar* tmp = NULL;
        gint i;

        for(i = 0; i < n_active; i++)
            if (str) {
                tmp = str;          
                str = g_strconcat(str, " | ", states[i], NULL);
                g_free(tmp);
            } else {
                str = g_strdup(states[i]);
            }

        tmp = str;
        str = g_strconcat(str, "\n", NULL);
        g_free(tmp);

        return str;
    }
}
