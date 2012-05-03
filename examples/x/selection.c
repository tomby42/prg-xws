/* Demonstrace fungování selections */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#include "Xlib_icon.h"

/*
 * Globálnì pou¾ívané promìnné
 */
char *progname; /* Jméno programu z argv[0] */

char *display_name; /* Jméno pou¾ívaného displeje */
Display *display; /* Struktura popisující displej */
int screen_num; /* Èíslo obrazovky, kterou bude program pou¾ívat */
Screen *screen; /* Struktura popisující obrazovku */
unsigned int screen_width, screen_height; /* Rozmìry obrazovky v pixelech */
unsigned int screen_width_mm, screen_height_mm; /* Rozmìry obrazovky v mm */
Window root; /* Root okno */
unsigned long black, white; /* Hodnoty BlackPixel a WhitePixel */

Window topwin; /* Top-level okno */
Pixmap topwin_icon; /* Pixmapa pro ikonu top-level okna */
int topwin_x, topwin_y; /* Pozice okna */
unsigned int topwin_w, topwin_h, topwin_b; /* Rozmìry okna a okraje */

GC def_gc; /* Standardní grafický kontext */
GC my_gc; /* Grafický kontext, vytvoøený se standardními hodnotami */
XFontStruct *def_font; /* Informace o standardním fontu */

Atom targets; /* Atom pro "TARGETS" */
Atom sel_prop; /* Property pro data z výbìru */

char *text = NULL; /* Vypisovaný text */

int do_debug = 1; /* Vypisovat ladicí hlá¹ky */

/*
 * Funkce pro výpis ladicích hlá¹ek
 */
static void debug(const char *msg, ...)
{
    va_list args;
    if(!do_debug)
	return;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

/*
 * Pøipojit k serveru, zjistit o nìm informace
 */
static void open_display(void)
{
    Atom val;
    char *fname;
    
    display_name = XDisplayName(NULL);
    if(!(display = XOpenDisplay(display_name))) {
	fprintf(stderr, "Cannot connect to X server \"%s\"\n", display_name);
	exit(1);
    }
    debug("Connected to X server \"%s\"\n", display_name);
    
    screen_num = DefaultScreen(display);
    screen = DefaultScreenOfDisplay(display);
    debug("Default screen number is %d\n", screen_num);
    
    screen_width = DisplayWidth(display, screen_num);
    screen_height = DisplayHeight(display, screen_num);
    screen_width_mm = DisplayWidthMM(display, screen_num);
    screen_height_mm = DisplayHeightMM(display, screen_num);
    debug("Screen size is %ux%u pixels, %ux%u mm\n", screen_width,
	  screen_height, screen_width_mm, screen_height_mm);
    
    root = RootWindow(display, screen_num);
    black = BlackPixel(display, screen_num);
    white = WhitePixel(display, screen_num);
    debug("Root window ID is %#lx, black is %#lx, white is %#lx\n", root,
	  black, white);

    def_gc = DefaultGC(display, screen_num);
    /* Toto volání XQueryFont() do def_font->fid neulo¾í ID fontu, ale
     * grafického kontextu! Nevím, jak získat ID standardního fontu. */
    if(!(def_font = XQueryFont(display, XGContextFromGC(def_gc)))) {
	fprintf(stderr, "XQueryFont() failed\n");
	exit(1);
    }
    if(XGetFontProperty(def_font, XA_FONT, &val)) {
	fname = XGetAtomName(display, val);
	debug("Default font name: %s\n", fname);
	XFree(fname);
    } else
	fprintf(stderr, "Cannot obtain default font name\n");
}

/*
 * Vytvoøit a namapovat top-level okno, vytvoøit dal¹í zdroje (napø. GC)
 */
static void create_resources(int argc, char *argv[])
{
    XWMHints *wm_hints; /* Doporuèení pro window manager */
    XSizeHints *size_hints; /* Doporuèená velikost okna */
    XClassHint *class_hint; /* Jméno a tøída pro resource manager */
    XTextProperty window_name, icon_name; /* Jména pro window manager */
    XGCValues gc_values; /* Hodnoty pro nastavení GC */
    char *str_window_name = (char *)"Hello Window"; /* Titulek okna */
    char *str_icon_name = (char *)"Hello Icon"; /* Titulek ikony */
   
    /* Vytvoøit top-level okno */
    topwin_x = topwin_y = 0; /* Okno umístí window manager */
    topwin_w = screen_width / 2;
    topwin_h = screen_height / 2;
    topwin_b = 0;
    topwin = XCreateSimpleWindow(display, root, topwin_x, topwin_y, topwin_w,
				 topwin_h, topwin_b, black, white);
    debug("Created top-level window ID %#lx\n", topwin);

    /* Vytvoøit ikonu pro top-level okno */
    topwin_icon = XCreateBitmapFromData(display, topwin, Xlib_icon_bits,
					Xlib_icon_width, Xlib_icon_height);
    debug("Created icon %#lx for top-level window\n", topwin_icon);

    /* Nastavit properties pro window manager */
    if(!(size_hints = XAllocSizeHints()) ||
       !(wm_hints = XAllocWMHints()) ||
       !(class_hint = XAllocClassHint())) {
	fprintf(stderr, "Cannot allocate memory\n");
	exit(1);
    }
    
    /*  Poloha a velikost se bere ze skuteèných hodnot okna v oka¾iku
     * namapování. PPosition | PSize øíká, ¾e hodnoty nastavil program (a
     * window manager je mù¾e mìnit podle potøeby). USSize | USPosition by
     * znamenalo, ¾e hodnoty zadal u¾ivatel (napø. na pøíkazové øádce) a
     * window manager by je nemìl mìnit. */
    size_hints->flags = PPosition | PSize | PMinSize;
    /* Window manager by nemìl okno nikdy zmen¹it na ménì ne¾
     * min_width x min_height pixelù. */
    size_hints->min_width = 200;
    size_hints->min_height = 100;
    
    wm_hints->initial_state = NormalState;/* Na zaèátku má být zobrazeno
                                             normální okno (ne ikona) */
    wm_hints->input = True; /* Aplikace potøebuje vstup z klávesnice */
    wm_hints->icon_pixmap = topwin_icon;
    wm_hints->flags = StateHint | InputHint | IconPixmapHint;
    
    /* Ulo¾ení jména okna a ikony */
    if(!XStringListToTextProperty(&str_window_name, 1, &window_name) ||
       !XStringListToTextProperty(&str_icon_name, 1, &icon_name)) {
        fprintf(stderr, "XStringListToTextProperty() for failed\n");
        exit(1);
    }

    /* Jméno programu a tøídy pro hledání v resource databázi */
    class_hint->res_name = progname;
    class_hint->res_class = (char *) "HelloWorld";

    /* Nastavit v¹echny properties */
    XSetWMProperties(display, topwin, &window_name, &icon_name, argv, argc,
                     size_hints, wm_hints, class_hint);

    /* Vybrat události pro top-level okno */
    XSelectInput(display, topwin, ExposureMask | KeyPressMask |
		 ButtonPressMask | StructureNotifyMask);
    
    /* Vytvoøit GC */
    gc_values.background = white;
    gc_values.foreground = black;
    my_gc = XCreateGC(display, topwin, GCBackground | GCForeground, &gc_values);
    XCopyGC(display, def_gc, GCFont, my_gc); /* default font */
    
    /* Získat atomy */
    targets = XInternAtom(display, "TARGETS", False);
    sel_prop = XInternAtom(display, "SEL_PROP", False);
   
    /* Namapovat top-level okno */
    XMapWindow(display, topwin);
}

/*
 * Kreslení
 */
static void expose(XExposeEvent *event)
{
    if(event->count > 0)
	return; /* Kreslit jen po poslední události v øadì */
    if(text)
	XDrawString(display, topwin, my_gc, 10, (int)topwin_h / 2,
		    text, (int)strlen(text));
}

/*
 * Hlavní cyklus zpracování událostí
 */
static void event_loop(void)
{
    XEvent event;
    XSelectionEvent sel;
    Atom tgt_atoms[] = { targets, XA_STRING };
    Atom gwp_type;
    int gwp_format;
    unsigned long gwp_items, gwp_after;
    unsigned char *gwp_prop;
    
    while(1) {
	XNextEvent(display, &event);
	switch(event.type) {
	    case Expose:
		expose(&event.xexpose);
		break;
	    case ButtonPress:
		debug("Button %u pressed\n", event.xbutton.button);
		switch(event.xbutton.button) {
		    case 1: /* Pøivlastnìní výbìru */
			XSetSelectionOwner(display, XA_PRIMARY, topwin,
					   event.xbutton.time);
			if(XGetSelectionOwner(display, XA_PRIMARY) == topwin)
			    debug("Selection aquired\n");
			else
			    debug("Selection not aquired\n");
			break;
		    case 2: /* Pøeètení dat z výbìru */
			XConvertSelection(display, XA_PRIMARY, XA_STRING,
					  sel_prop, topwin, event.xbutton.time);
			break;
		}
		break;
	    case KeyPress:
		return;
		break;
	    case SelectionClear: /* Nìkdo jiný si pøivlastnil výbìr */
		debug("Selection ownership taken away\n");
		break;
	    case SelectionNotify: /* Vlastník poslal data */
		if(event.xselection.target != XA_STRING)
		    debug("Received unexpected target %s\n",
			  XGetAtomName(display, event.xselection.target));
		else if(event.xselection.property != sel_prop)
		    debug("Owner cannot provide selection\n");
		else
		    if(XGetWindowProperty(display, topwin, sel_prop, 0, 
					  LONG_MAX, True, AnyPropertyType,
					  &gwp_type, &gwp_format, &gwp_items,
					  &gwp_after, &gwp_prop) == Success) {
			if(gwp_type != XA_STRING || gwp_format != 8) {
			    fprintf(stderr, "Unexpected type %s, format %d\n",
				    XGetAtomName(display, gwp_type), 
				    gwp_format);
			    XFree(gwp_prop);
			} else {
			    if(text)
				XFree(text);
			    text = gwp_prop;
			    XClearArea(display, topwin, 0, 0, 0, 0, True);
			}
		    } else
			fprintf(stderr, "XGetWindowProperty() failed\n");
		break;
	    case SelectionRequest: /* Nìkdo chce obsah výbìru */
		sel.type = SelectionNotify;
		sel.requestor = event.xselectionrequest.requestor;
		sel.selection = XA_PRIMARY;
		sel.target = event.xselectionrequest.target;
		sel.property = event.xselectionrequest.property;
		sel.time = event.xselectionrequest.time;
		if(sel.target == XA_STRING) {
		    debug("Providing \"STRING\" to %#x\n", sel.requestor);
		    XChangeProperty(display, sel.requestor, sel.property,
				    XA_STRING, 8, PropModeReplace,
				    (char *)"Hello World", 11);
		} 
		else if(sel.target == targets) {
		    debug("Providing \"TARGETS\" to %#x\n", sel.requestor);
		    XChangeProperty(display, sel.requestor, sel.property,
				    XA_ATOM, sizeof(Atom)*8, PropModeReplace,
				    (char *)&tgt_atoms, 2);
		}
		else {
		    debug("Cannot convert selection to target %s "
			  "requested by %#x\n",
			  XGetAtomName(display, sel.target), sel.requestor);
		    sel.property = None;
		}
		XSendEvent(display, sel.requestor, False, 0, (XEvent *)&sel);
		break;
	    case ConfigureNotify:
		if((int)topwin_w != event.xconfigure.width ||
		   (int)topwin_h != event.xconfigure.height) {
		    topwin_w = event.xconfigure.width;
		    topwin_h = event.xconfigure.height;
		    debug("Window resized to %ux%u pixels\n",
			  topwin_w, topwin_h);
		    XClearWindow(display, topwin);
		}
		break;
	    case MappingNotify:
		/* Naèíst zmìnìné mapování kláves */
		if(event.xmapping.request == MappingKeyboard)
		    XRefreshKeyboardMapping(&event.xmapping);
		break;
	    default:
		/* Zbylé nezajímavé události */
		break;
	}
    }
}

/*
 * Odpojit od X serveru
 */
static void close_display(void)
{
    /* Po odpojení klienta X server automaticky zru¹í v¹echny jím vytvoøené
     * zdroje. XCloseDisplay() voláme hlavnì proto, abychom dostali pøípadné
     * chyby, které jsem zatím nedostali. */
    XCloseDisplay(display);
}

/*
 * Vstupní bod programu
 */
int main(int argc, char *argv[])
{
    progname = argv[0];

    open_display();
    create_resources(argc, argv);
    event_loop();
    close_display();
   
    return 0;
}
