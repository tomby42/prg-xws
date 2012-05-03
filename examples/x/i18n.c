/* Uk�zka internacionalizace */

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

XFontSet fontset; /* Font set pro zobrazov�n� textu */
char *font_name = (char *)"-misc-fixed-*-*-*-*-*-130-75-75-*-*-*-*"; /* Jm�no fontu */
wchar_t text[STR_LEN+1]; /* Buffer pro zad�van�/vypisovan� text */
int text_len = 0; /* Po�et znak� v text */

XIM xim; /* Vstupn� metoda */
XIC xic; /* Vstupn� kontext */

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
    /* Toto vol�n� XQueryFont() do def_font->fid neulo�� ID fontu, ale
     * grafick�ho kontextu! Nev�m, jak z�skat ID standardn�ho fontu. */
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
    char **mis_chars; /* Chyb�j�c� charsety ve fontset */
    int num_mis_chars = 0; /* Po�et chyb�j�c�ch charset� */
    char *def_str; /* �et�zec vypisovan� m�sto chyb�j�c�ch znak� */
    int num_fonts; /* Po�et font� ve fontset */
    XFontStruct **fs_fonts; /* Fonty ve fontset */
    char **fs_names; /* Jm�na font� ve fontset */
    XIMStyles *im_styles; /* Styly vstupn� metody */
    XIMStyle my_style, style; /* Vybran� a aktu�ln� testovan� styl */
    char str_style[300], str_my_style[300]; /* Textov� z�pis stylu */
    XVaNestedList list; /* Parametry pro vytvo�en� vstupn�ho kontextu */
    int i;
   
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
    
    /* Vytvo�it font set */
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
    
    /* Vytvo�it GC */
    gc_values.background = white;
    gc_values.foreground = black;
    my_gc = XCreateGC(display, topwin, GCBackground | GCForeground, &gc_values);
    XCopyGC(display, def_gc, GCFont, my_gc); /* default font */
    
    /* Otev��t vstupn� metodu, zjistit vhodn� styl */
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
	    /* Vybereme si ze styl�, kter� um�me */
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

    /* Vytvo�it vstupn� kontext */
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
 * Kreslen�
 */
static void expose(XExposeEvent *event)
{
    if(event->count > 0)
	return; /* Kreslit jen po posledn� ud�losti v �ad� */
    XwcDrawString(display, topwin, fontset, my_gc, 10, (int)topwin_h / 2, text, text_len);
}

/*
 * Hlavn� cyklus zpracov�n� ud�lost�
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
	if(XFilterEvent(&event, None)) /* Filtrace ud�lost� vstupn� metodou */
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
   
    /* Jm�no fontu z argumentu 1 */
    if(argc >= 2)
	font_name = argv[1];
    
    open_display();
    create_resources(argc, argv);
    event_loop();
    close_display();
   
    return 0;
}
