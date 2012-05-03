/* Ukázka internacionalizace */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#include "Xlib_icon.h"

#define STR_LEN 128

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

XFontSet fontset; /* Font set pro zobrazování textu */
char *font_name = (char *)"-misc-fixed-*-*-*-*-*-130-75-75-*-*-*-*"; /* Jméno fontu */
wchar_t text[STR_LEN+1]; /* Buffer pro zadávaný/vypisovaný text */
int text_len = 0; /* Poèet znakù v text */

XIM xim; /* Vstupní metoda */
XIC xic; /* Vstupní kontext */

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
    char **mis_chars; /* Chybìjící charsety ve fontset */
    int num_mis_chars = 0; /* Poèet chybìjících charsetù */
    char *def_str; /* Øetìzec vypisovaný místo chybìjících znakù */
    int num_fonts; /* Poèet fontù ve fontset */
    XFontStruct **fs_fonts; /* Fonty ve fontset */
    char **fs_names; /* Jména fontù ve fontset */
    XIMStyles *im_styles; /* Styly vstupní metody */
    XIMStyle my_style, style; /* Vybraný a aktuálnì testovaný styl */
    char str_style[300], str_my_style[300]; /* Textový zápis stylu */
    XVaNestedList list; /* Parametry pro vytvoøení vstupního kontextu */
    int i;
   
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
    
    /* Vytvoøit font set */
    fontset = XCreateFontSet(display, font_name, &mis_chars, &num_mis_chars, &def_str);
    if(num_mis_chars > 0) {
	fprintf(stderr, "%d charsets missing:\n", num_mis_chars);
	for(i = 0; i < num_mis_chars; i++)
	    fprintf(stderr, "  %s\n", mis_chars[i]);
	fprintf(stderr, "Replacement for missing characters: \"%s\"\n", def_str);
    }
    if(!fontset) {
	fprintf(stderr, "XCreateFontSet() returned NULL\n");
	exit(1);
    }
    XFreeStringList(mis_chars);
    num_fonts = XFontsOfFontSet(fontset, &fs_fonts, &fs_names);
    fprintf(stderr, "%d fonts in fontset:\n", num_fonts);
    for(i = 0; i < num_fonts; i++)
	fprintf(stderr, "  %s\n", fs_names[i]);
    fprintf(stderr, "Locale of fontset: %s\n", XLocaleOfFontSet(fontset));
    
    /* Vytvoøit GC */
    gc_values.background = white;
    gc_values.foreground = black;
    my_gc = XCreateGC(display, topwin, GCBackground | GCForeground, &gc_values);
    XCopyGC(display, def_gc, GCFont, my_gc); /* default font */
    
    /* Otevøít vstupní metodu, zjistit vhodný styl */
    if(!(xim = XOpenIM(display, NULL, NULL, NULL))) {
	fprintf(stderr, "Cannot open input method\n");
	exit(1);
    }
    XGetIMValues(xim, XNQueryInputStyle, &im_styles, NULL);
    my_style = 0;
    str_my_style[0] = '\0';
    fprintf(stderr, "Input method supports %hu styles:\n", im_styles->count_styles);
    for(i = 0; i < im_styles->count_styles; i++) {
	style = im_styles->supported_styles[i];
	str_style[0] = '\0';
	if(style & XIMPreeditCallbacks)
	    strcat(str_style, "XIMPreeditCallbacks | ");
	if(style & XIMPreeditPosition)
	    strcat(str_style, "XIMPreeditPosition | ");
	if(style & XIMPreeditArea)
	    strcat(str_style, "XIMPreeditArea | ");
	if(style & XIMPreeditNothing)
	    strcat(str_style, "XIMPreeditNothing | ");
	if(style & XIMPreeditNone)
	    strcat(str_style, "XIMPreeditNone | ");
	if(style & XIMStatusCallbacks)
	    strcat(str_style, "XIMStatusCallbacks | ");
	if(style & XIMStatusArea)
	    strcat(str_style, "XIMStatusArea | ");
	if(style & XIMStatusNothing)
	    strcat(str_style, "XIMStatusNothing | ");
	if(style & XIMStatusNone)
	    strcat(str_style, "XIMStatusNone | ");
	if(str_style[0] != '\0')
	    str_style[strlen(str_style)-3] = '\0';
	fprintf(stderr, "  %s\n", str_style);
	if((style &
	   (XIMPreeditNothing|XIMPreeditNone|XIMStatusNothing|XIMStatusNone)) == style) {
	    /* Vybereme si ze stylù, které umíme */
	    if(my_style == 0) {
		my_style = style;
	    } else {
		if(my_style & XIMPreeditNone) {
		    if((style & XIMPreeditNothing) || (style & XIMStatusNothing))
			my_style = style;
		} else {
		    if((style & XIMPreeditNothing) && (style & XIMStatusNothing))
			my_style = style;
		}
	    }
	    if(style == my_style)
		strcpy(str_my_style, str_style);
	}
    }
    XFree(im_styles);
    fprintf(stderr, "Will use style %s\n", str_my_style);

    /* Vytvoøit vstupní kontext */
    list = XVaCreateNestedList(0, XNFontSet, fontset, NULL);
    xic = XCreateIC(xim, XNInputStyle, my_style, XNClientWindow, topwin,
		    XNPreeditAttributes, list, XNStatusAttributes, list, NULL);
    XFree(list);
    if(!xic) {
	fprintf(stderr, "Cannot create input context\n");
	exit(1);
    }
    XSetICFocus(xic);
   
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
    XwcDrawString(display, topwin, fontset, my_gc, 10, (int)topwin_h / 2, text, text_len);
}

/*
 * Hlavní cyklus zpracování událostí
 */
static void event_loop(void)
{
    XEvent event;
    wchar_t buf[32];
    KeySym keysym;
    Status status;
    size_t ret;
    XTextProperty wname;
    wchar_t *p = text;
    
    while(1) {
	XNextEvent(display, &event);
	if(XFilterEvent(&event, None)) /* Filtrace událostí vstupní metodou */
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
		ret = XwcLookupString(xic, &event.xkey, buf, 32, &keysym, &status);
		if(status == XLookupChars || status == XLookupBoth) {
		    debug("KeyPress -> char\n");
		    if(text_len + ret <= STR_LEN && ret >= 1 && iswprint((wint_t)buf[0])) {
			wmemcpy(text + text_len, buf, ret);
			text_len += ret;
			text[text_len] = 0;
			XClearArea(display, topwin, 0, 0, 0, 0, True);
		    }
		}
		if(status == XLookupKeySym || status == XLookupBoth) {
		    debug("KeyPress -> keysym \"%s\"\n", XKeysymToString(keysym));
		    switch(keysym) {
			case XK_BackSpace:
			    if(text_len > 0) {
				text[--text_len] = 0;
				XClearArea(display, topwin, 0, 0, 0, 0, True);
			    }
			    break;
			case XK_Return:
			    XwcTextListToTextProperty(display, &p, 1, XTextStyle, &wname);
			    XSetWMName(display, topwin, &wname);
			    break;
			default:
			    break;
		    }
		}
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
    char *locale;
    
    progname = argv[0];

    /* Nastavit locale */
    if(!(locale = setlocale(LC_ALL, ""))) {
	fprintf(stderr, "Cannot set locale\n");
	exit(1);
    }
    debug("setlocale() returned \"%s\"\n", locale);
    if(!XSupportsLocale()) {
	fprintf(stderr, "X does not support locale\n");
	exit(1);
    }
    if(!(locale=XSetLocaleModifiers("")))
	fprintf(stderr, "Warning: cannot set locale modifiers\n");
    debug("XSetLocaleModifiers() returned \"%s\"\n", locale);
   
    /* Jméno fontu z argumentu 1 */
    if(argc >= 2)
	font_name = argv[1];
    
    open_display();
    create_resources(argc, argv);
    event_loop();
    close_display();
   
    return 0;
}
