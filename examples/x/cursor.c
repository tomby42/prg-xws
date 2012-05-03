/* Nastavení kurzoru */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h> /* Konstanty pro XCreateFontCursor() */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "Xlib_icon.h"
#include "cursor_source.h"
#include "cursor_mask.h"

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


Window topwin, win1, win2; /* Okna */
Pixmap topwin_icon; /* Pixmapa pro ikonu top-level okna */
int topwin_x, topwin_y; /* Pozice okna */
unsigned int topwin_w, topwin_h, topwin_b; /* Rozmìry okna a okraje */

GC def_gc; /* Standardní grafický kontext */
GC my_gc; /* Grafický kontext, vytvoøený se standardními hodnotami */
XFontStruct *def_font; /* Informace o standardním fontu */

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
    if(!(def_font = XQueryFont(display, XGContextFromGC(def_gc)))) {
	fprintf(stderr, "XQueryFont() failed\n");
	exit(1);
    }
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
    Cursor cursor; /* Kurzor */
    XSetWindowAttributes set_attr;
    XColor bg, fg; /* Barvy pro kurzor */
    Pixmap source, mask; /* Bitmapy pro kurzor */
    int cw, ch; /* Velikost kurzoru */
    int qbc; /* Návratová hodnota XQueryBestCursor() */
    Font font; /* Font pro vytvoøení kurzoru */
   
    /* Vytvoøit top-level okno */
    topwin_x = topwin_y = 0; /* Okno umístí window manager */
    topwin_w = screen_width / 2;
    topwin_h = screen_height / 2;
    topwin_b = 0;
    topwin = XCreateSimpleWindow(display, root, topwin_x, topwin_y, topwin_w,
				 topwin_h, topwin_b, black, white);
    debug("Created top-level window ID %#lx\n", topwin);

    /* Pøipravit barvy pro kurzor */
    fg.red = 0x8000;
    fg.green = fg.blue = 0;
    bg.green = 0xffff;
    bg.red = bg.blue = 0;
    
    /* Nastavit kurzor top-level okna */
    cursor = XCreateFontCursor(display, XC_cross);
    XRecolorCursor(display, cursor, &fg, &bg);
    XDefineCursor(display, topwin, cursor);

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
    XSelectInput(display, topwin, KeyPressMask | ButtonPressMask);
    
    /* Vytvoøit dal¹í okna */
    qbc = XQueryBestCursor(display, topwin, cursor_source_width, 
	      		   cursor_source_height, &cw, &ch);
    debug("XQueryBestCursor(%d, %d) = %d -> %dx%d\n", cursor_source_width,
	  cursor_source_height, qbc, cw, ch);
    source = XCreateBitmapFromData(display, topwin, cursor_source_bits,
			   	   cursor_source_width, cursor_source_height);
    mask = XCreateBitmapFromData(display, topwin, cursor_mask_bits,
				 cursor_mask_width, cursor_mask_height);
    set_attr.cursor = XCreatePixmapCursor(display, source, mask, &fg, &bg,
					  4, 4);
    win1 = XCreateWindow(display, topwin, 20, 10, 150, 75, 0, CopyFromParent,
			 InputOnly, CopyFromParent, CWCursor, &set_attr);

    font = XLoadFont(display, "fixed");
    set_attr.cursor = XCreateGlyphCursor(display, font, font, '+', '*',
					 &fg, &bg);
    win2 = XCreateWindow(display, topwin, 200, 10, 150, 75, 0, CopyFromParent,
			 InputOnly, CopyFromParent, CWCursor, &set_attr);
    
    /* Vytvoøit GC */
    gc_values.background = white;
    gc_values.foreground = black;
    my_gc = XCreateGC(display, topwin, GCBackground | GCForeground, &gc_values);
    XCopyGC(display, def_gc, GCFont, my_gc); /* default font */
    
    /* Namapovat okna */
    XMapWindow(display, win1);
    XMapWindow(display, win2);
    XMapWindow(display, topwin);
}

/*
 * Hlavní cyklus zpracování událostí
 */
static void event_loop(void)
{
    XEvent event;
    char buf[10];
    KeySym keysym;
    
    while(1) {
	XNextEvent(display, &event);
	switch(event.type) {
	    case ButtonPress:
		XDrawPoint(display, topwin, def_gc, event.xbutton.x,
			   event.xbutton.y);
		break;
	    case KeyPress:
		XLookupString(&event.xkey, buf, 10, &keysym, NULL);
		debug("Key \"%s\" pressed\n", XKeysymToString(keysym));
		return;
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
