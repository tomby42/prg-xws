/* Uk�zka o�ez�v�n� (clipping) */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "Xlib_icon.h"

/*
 * Glob�ln� pou��van� prom�nn�
 */
char *progname; /* Jm�no programu z argv[0] */

char *display_name; /* Jm�no pou��van�ho displeje */
Display *display; /* Struktura popisuj�c� displej */
int screen_num; /* ��slo obrazovky, kterou bude program pou��vat */
Screen *screen; /* Struktura popisuj�c� obrazovku */
unsigned int screen_width, screen_height; /* Rozm�ry obrazovky v pixelech */
unsigned int screen_width_mm, screen_height_mm; /* Rozm�ry obrazovky v mm */
Window root; /* Root okno */
unsigned long black, white; /* Hodnoty BlackPixel a WhitePixel */

Window topwin; /* Top-level okno */
Pixmap topwin_icon; /* Pixmapa pro ikonu top-level okna */
int topwin_x, topwin_y; /* Pozice okna */
unsigned int topwin_w, topwin_h, topwin_b; /* Rozm�ry okna a okraje */

GC def_gc; /* Standardn� grafick� kontext */
GC my_gc; /* Grafick� kontext, vytvo�en� se standardn�mi hodnotami */
XFontStruct *def_font; /* Informace o standardn�m fontu */

int do_debug = 1; /* Vypisovat ladic� hl�ky */

/*
 * Funkce pro v�pis ladic�ch hl�ek
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
 * P�ipojit k serveru, zjistit o n�m informace
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
    /* Toto vol�n� XQueryFont() do def_font->fid neulo�� ID fontu, ale
     * grafick�ho kontextu! Nev�m, jak z�skat ID nebo jm�no standardn�ho
     * fontu. */
    if(!(def_font = XQueryFont(display, XGContextFromGC(def_gc)))) {
	fprintf(stderr, "XQueryFont() failed\n");
	exit(1);
    }
}

/*
 * Vytvo�it a namapovat top-level okno, vytvo�it dal�� zdroje (nap�. GC)
 */
static void create_resources(int argc, char *argv[])
{
    XWMHints *wm_hints; /* Doporu�en� pro window manager */
    XSizeHints *size_hints; /* Doporu�en� velikost okna */
    XClassHint *class_hint; /* Jm�no a t��da pro resource manager */
    XTextProperty window_name, icon_name; /* Jm�na pro window manager */
    XGCValues gc_values; /* Hodnoty pro nastaven� GC */
    char *str_window_name = (char *)"Hello Window"; /* Titulek okna */
    char *str_icon_name = (char *)"Hello Icon"; /* Titulek ikony */
   
    /* Vytvo�it top-level okno */
    topwin_x = topwin_y = 0; /* Okno um�st� window manager */
    topwin_w = screen_width / 2;
    topwin_h = screen_height / 2;
    topwin_b = 0;
    topwin = XCreateSimpleWindow(display, root, topwin_x, topwin_y, topwin_w,
				 topwin_h, topwin_b, black, white);
    debug("Created top-level window ID %#lx\n", topwin);

    /* Vytvo�it ikonu pro top-level okno */
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
    
    /*  Poloha a velikost se bere ze skute�n�ch hodnot okna v oka�iku
     * namapov�n�. PPosition | PSize ��k�, �e hodnoty nastavil program (a
     * window manager je m��e m�nit podle pot�eby). USSize | USPosition by
     * znamenalo, �e hodnoty zadal u�ivatel (nap�. na p��kazov� ��dce) a
     * window manager by je nem�l m�nit. */
    size_hints->flags = PPosition | PSize | PMinSize;
    /* Window manager by nem�l okno nikdy zmen�it na m�n� ne�
     * min_width x min_height pixel�. */
    size_hints->min_width = 200;
    size_hints->min_height = 100;
    
    wm_hints->initial_state = NormalState;/* Na za��tku m� b�t zobrazeno
                                             norm�ln� okno (ne ikona) */
    wm_hints->input = True; /* Aplikace pot�ebuje vstup z kl�vesnice */
    wm_hints->icon_pixmap = topwin_icon;
    wm_hints->flags = StateHint | InputHint | IconPixmapHint;
    
    /* Ulo�en� jm�na okna a ikony */
    if(!XStringListToTextProperty(&str_window_name, 1, &window_name) ||
       !XStringListToTextProperty(&str_icon_name, 1, &icon_name)) {
        fprintf(stderr, "XStringListToTextProperty() for failed\n");
        exit(1);
    }

    /* Jm�no programu a t��dy pro hled�n� v resource datab�zi */
    class_hint->res_name = progname;
    class_hint->res_class = (char *) "HelloWorld";

    /* Nastavit v�echny properties */
    XSetWMProperties(display, topwin, &window_name, &icon_name, argv, argc,
                     size_hints, wm_hints, class_hint);

    /* Vybrat ud�losti pro top-level okno */
    XSelectInput(display, topwin, ExposureMask | KeyPressMask |
		 ButtonPressMask | StructureNotifyMask);
    
    /* Vytvo�it GC */
    gc_values.background = white;
    gc_values.foreground = black;
    my_gc = XCreateGC(display, topwin, GCBackground | GCForeground, &gc_values);
    XCopyGC(display, def_gc, GCFont, my_gc); /* default font */
    
    /* Namapovat top-level okno */
    XMapWindow(display, topwin);
}

/*
 * Kreslen�
 */
static void expose(XExposeEvent *event)
{
    Region region, region1, region2;
    XRectangle rect;
    
    if(event->count > 0)
	return; /* Kreslit jen po posledn� ud�losti v �ad� */
  
    XDrawArc(display, topwin, my_gc, 20, 20, topwin_w - 40, topwin_h - 40,
	     0, 360 * 64);
    
    region = XCreateRegion();
    region1 = XCreateRegion();
    region2 = XCreateRegion();
    rect.x = rect.y = 0;
    rect.width = 100;
    rect.height = topwin_w - 60;
    XUnionRectWithRegion(&rect, region, region1);
    rect.width = 20;
    rect.height = 10;
    XUnionRectWithRegion(&rect, region2, region);
    XOffsetRegion(region, 50, 100);
    XSubtractRegion(region1, region, region2);
    XDestroyRegion(region);
    region = XCreateRegion();
    rect.width = 100;
    rect.height = 30;
    XUnionRectWithRegion(&rect, region, region1);
    XOffsetRegion(region1, (int)topwin_w - 120, -30 + ((int)topwin_h - 40) / 2);
    XUnionRegion(region1, region2, region);
    XSetRegion(display, my_gc, region);
    XDestroyRegion(region);
    XDestroyRegion(region1);
    XDestroyRegion(region2);
    XSetClipOrigin(display, my_gc, 20, 20);
    XFillArc(display, topwin, my_gc, 20, 20, topwin_w - 40, topwin_h - 40,
	     0, 360 * 64);
    XSetClipMask(display, my_gc, None); /* Zru�it o�ez�v�n� */
}

/*
 * Hlavn� cyklus zpracov�n� ud�lost�
 */
static void event_loop(void)
{
    XEvent event;
    char buf[10];
    KeySym keysym;
    
    while(1) {
	XNextEvent(display, &event);
	switch(event.type) {
	    case Expose:
		expose(&event.xexpose);
		break;
	    case ButtonPress:
		debug("Button %u pressed\n", event.xbutton.button);
		return;
		break;
	    case KeyPress:
		XLookupString(&event.xkey, buf, 10, &keysym, NULL);
		debug("Key \"%s\" pressed\n", XKeysymToString(keysym));
		return;
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
		/* Na��st zm�n�n� mapov�n� kl�ves */
		if(event.xmapping.request == MappingKeyboard)
		    XRefreshKeyboardMapping(&event.xmapping);
		break;
	    default:
		/* Zbyl� nezaj�mav� ud�losti */
		break;
	}
    }
}

/*
 * Odpojit od X serveru
 */
static void close_display(void)
{
    /* Po odpojen� klienta X server automaticky zru�� v�echny j�m vytvo�en�
     * zdroje. XCloseDisplay() vol�me hlavn� proto, abychom dostali p��padn�
     * chyby, kter� jsem zat�m nedostali. */
    XCloseDisplay(display);
}

/*
 * Vstupn� bod programu
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
