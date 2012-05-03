/* Vytvoøení palety a alokace barev */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "Xlib_icon.h"

#define PATCH_SIZE 4 /* velikost barevné plo¹ky */
#define WAIT_SEC 0 /* interval mezi úpravami palety (s) */
#define WAIT_USEC 50000 /* interval mezi úpravami palety (us) */
/*
 * Typ barev alokovaných v paletì
 */
enum colors {
    col_ro, /* read-only barvy */
    col_rw, /* read-write barvy */
    col_rorev /* read-only barvy v pøevráceném poøadí */
};

/*
 * Globálnì pou¾ívané promìnné
 */
char *progname; /* Jméno programu z argv[0] */

char *display_name; /* Jméno pou¾ívaného displeje */
Display *display; /* Struktura popisující displej */
int display_fd; /* File descriptor spojení se serverem */
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
enum colors colors; /* Jaké barvy se budou alokovat */
XVisualInfo visual; /* Maska pro hledaný visual a nalezený visual */
Colormap colormap; /* Paleta */
int shift_red, shift_green, shift_blue; /* Pozice masek R, G, B */
int bits_red, bits_green, bits_blue; /* Poèet bitù na slo¾ky R, G, B */

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
    display_fd = ConnectionNumber(display);
    debug("Connected to X server \"%s\", descriptor %d\n", display_name,
	  display_fd);
    
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
    XVisualInfo *vinfo; /* Seznam nalezených visuals */
    int vnum; /* Poèet vyhovujících visuals */
    int i, vi, vdepth;
    XSetWindowAttributes setattr;
    XColor color; /* Alokovaná barva */
    unsigned long *pixels; /* Pole alokovaných polo¾el palety */
    unsigned long mask_red, mask_green, mask_blue;
   
    /* Zjistit visuals */
    visual.screen = screen_num;
    visual.class = DirectColor; /* tøíslo¾ková read/write paleta */
    vinfo = XGetVisualInfo(display, VisualScreenMask | VisualClassMask,
			   &visual, &vnum);
    if(vnum == 0) {
	debug("No suitable visual found\n");
	exit(1);
    } else
	debug("%d matching visuals found\n", vnum);
    vdepth = 0; /* Najít visual s nejvìt¹í hloubkou */
    for(i = 0; i < vnum; i++) {
	debug("Id %#x, depth %u\n", vinfo[i].visualid, vinfo[i].depth);
	if(vinfo[i].depth > vdepth) {
	    vdepth = vinfo[i].depth;
	    vi = i;
	}
    }
    visual = vinfo[vi];
    debug("Chosen visual %#x, bits_per_rgb %d, masks: red %#lx, green %#lx, blue %#lx\n",
	  visual.visualid, visual.bits_per_rgb, visual.red_mask,
	  visual.green_mask, visual.blue_mask);
    XFree(vinfo);
    shift_red = bits_red = 0;
    shift_green = bits_green = 0;
    shift_blue = bits_blue = 0;
    for(i = visual.red_mask; (i & 1) == 0; i >>= 1)
	shift_red++;
    for(; (i & 1) == 1; i >>= 1)
	bits_red++;
    for(i = visual.green_mask; (i & 1) == 0; i >>= 1)
	shift_green++;
    for(; (i & 1) ==1 ; i >>= 1)
	bits_green++;
    for(i = visual.blue_mask; (i & 1) == 0; i >>= 1)
	shift_blue++;
    for(; (i & 1) ==1 ; i >>= 1)
	bits_blue++;
    debug("colormap_size %d, red shift %d, bits %d, green shift %d, bits %d, "
	  " blue shift %d, bits %d\n", visual.colormap_size, shift_red,
	  bits_red, shift_green, bits_green, shift_blue, bits_blue);
    /* Vytvoøit paletu a alokovat barvy. Alokujeme v prázdné paletì, proto
     * pøedpokládáme, ¾e dostáváme polo¾ky postupnì od 0. */
    colormap = XCreateColormap(display, root, visual.visual, AllocNone);
    if(colors == col_rw) { /* alokovat read/write barvy */
	if(!(pixels = malloc(sizeof(*pixels) * (1 << bits_red) * 
			     (1 << bits_green)))) {
	    perror("malloc()");
	    exit(1);
	}
	if(!XAllocColorPlanes(display, colormap, True, pixels, 1, bits_red,
			      bits_green, 0, &mask_red, &mask_green,
			      &mask_blue)) {
	    fprintf(stderr, "Cannot allocate %d color cells\n",
		    (1 << bits_red) * (1 << bits_green));
	    exit(1);
	}
	debug("Color planes: mask_red %#lx, mask_green %#lx, mask_blue %#lx\n",
	      mask_red, mask_green, mask_blue);
	for(i = 0; i < (1 << bits_red); i++) {
	    color.red = i * (1 << (16 - bits_red));
	    color.flags = DoRed;
	    color.pixel = i << shift_red;
	    XStoreColor(display, colormap, &color);
	}
	for(i = 0; i < (1 << bits_green); i++) {
	    color.green = i * (1 << (16 - bits_green));
	    color.flags = DoGreen;
	    color.pixel =i << shift_green;
	    XStoreColor(display, colormap, &color);
	}
    } else { /* alokovat read-only barvy */
	for(i = 0; i < (1 << bits_red); i++) {
	    color.red = i * (1 << (16 - bits_red));
	    color.green = 0;
	    color.blue = 0;
	    if(colors == col_rorev)
		color.red = 65535 - color.red;
	    if(!XAllocColor(display, colormap, &color)) {
		fprintf(stderr, "Cannot allocate %dth color\n", i);
		exit(1);
	    }
	}
     	for(i = 0; i < (1 << bits_green); i++) {
	    color.red = 0;
	    color.green = i * (1 << (16 - bits_green));
	    color.blue = 0;
    	    if(colors == col_rorev)
		color.green = 65535 - color.green;
	    if(!XAllocColor(display, colormap, &color)) {
    		fprintf(stderr, "Cannot allocate %dth color\n", i);
		exit(1);
	    }
	}
    }
    
    /* Vytvoøit top-level okno */
    topwin_x = topwin_y = 0; /* Okno umístí window manager */
    topwin_w = (1 << bits_red) * PATCH_SIZE;
    topwin_h = (1 << bits_green) * PATCH_SIZE;
    topwin_b = 0;
    setattr.colormap = colormap;
    topwin = XCreateWindow(display, root, topwin_x, topwin_y, topwin_w,
			   topwin_h, topwin_b, visual.depth, InputOutput,
			   visual.visual, CWColormap, &setattr);
    debug("Created top-level window ID %#lx, %ux%u pixels\n", topwin,
	  topwin_w, topwin_h);

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
    my_gc = XCreateGC(display, topwin, 0, &gc_values);
    XCopyGC(display, def_gc, GCFont, my_gc); /* default font */
    
    /* Namapovat top-level okno */
    XMapWindow(display, topwin);
}

/*
 * Kreslení
 */
static void expose(XExposeEvent *event)
{
    int i, j;
    
    if(event->count > 0)
	return; /* Kreslit jen po poslední události v øadì */
    
    for(i = 0; i < (1 << bits_red); i++)
	for(j = 0; j < (1 << bits_green); j++) {
	    XSetForeground(display, my_gc, 
			   (unsigned)(i << shift_red) + (j << shift_green));
	    XFillRectangle(display, topwin, my_gc, i * PATCH_SIZE,
			   j * PATCH_SIZE, PATCH_SIZE, PATCH_SIZE);
	    if(i % (1 << bits_green) == j % (1 << bits_red))
		XFillArc(display, topwin, def_gc, i * PATCH_SIZE,
			 j* PATCH_SIZE, PATCH_SIZE, PATCH_SIZE, 0, 64 * 360);
	}
}

/*
 * Hlavní cyklus zpracování událostí
 */
static void event_loop(void)
{
    XEvent event;
    char buf[10];
    KeySym keysym;
    fd_set rfd;
    struct timeval timeout;
    XColor base_color, color;
    int i;
    
    while(1) {
	FD_ZERO(&rfd);
	FD_SET(display_fd, &rfd);
	timeout.tv_sec = WAIT_SEC;
	timeout.tv_usec = WAIT_USEC;
	if(XPending(display) == 0 &&
	   select(display_fd + 1, &rfd, NULL, NULL, &timeout) != 1) {
	    /* Upravit paletu */
	    if(colors == col_rw) {
		color.pixel = 0;
		XQueryColor(display, colormap, &base_color);
		/* Cyklus barev mù¾e být èásteènì posunutý, proto¾e z
		 * XQueryColor() dostaneme skuteènì pou¾itou barvu, co¾ nemusí
		 * být stejná hodnota, jakou jsme zadali, proto¾e server
		 * zaokrouhluje na nejbli¾¹í fyzicky zobrazitelnou barvu. */
		base_color.red =
		    (base_color.red + (1 << (16 - bits_red))) % 0xffff;
		base_color.green = (base_color.green +
				    (1 << (16 - bits_green))) % 0xffff;
		for(i = 0; i < (1 << bits_red); i++) {
		    color.red = (i * (1 << (16 - bits_red)) +
				 base_color.red) % 0xffff;
		    color.flags = DoRed;
		    color.pixel = i << shift_red;
		    XStoreColor(display, colormap, &color);
		}
	     	for(i = 0; i < (1 << bits_green); i++) {
    		    color.green = (i * (1 << (16 - bits_green)) +
				   base_color.green) % 0xffff;
		    color.flags = DoGreen;
		    color.pixel = i << shift_green;
		    XStoreColor(display, colormap, &color);
		}
	    }
	    continue;
	}
	if(!XCheckMaskEvent(display, ~(0L), &event))
	    continue;
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
 * Popis pou¾ití
 */
static void usage(void)
{
    fprintf(stderr, "usage: colormap {ro|rw|rorev}\n");
    exit(1);
}

/*
 * Vstupní bod programu
 */
int main(int argc, char *argv[])
{
    progname = argv[0];

    if(argc != 2)
	usage();
    if(!strcmp(argv[1], "ro"))
	colors = col_ro;
    else if(!strcmp(argv[1], "rw"))
	colors = col_rw;
    else if(!strcmp(argv[1], "rorev"))
	colors = col_rorev;
    else
	usage();
   
    open_display();
    create_resources(argc, argv);
    event_loop();
    close_display();
   
    return 0;
}
