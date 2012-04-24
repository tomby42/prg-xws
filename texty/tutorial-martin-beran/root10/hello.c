/* Jednoduch� program v X s pou�it�m pouze Xlib, bez toolkitu. Ilustruje
 * z�kladn� inicializaci X klienta, vytvo�en� okna, kreslen� a event loop. */

/* Z�kladn� deklarace pro Xlib */
#include <X11/Xlib.h>
/* Dal�� deklarace pro Xlib */
#include <X11/Xutil.h>
/* P�eddefinovan� atomy */
#include <X11/Xatom.h>
/* M�sto include pro �asto pou��van� syst�mov� hlavi�kov� soubory */
#include <X11/Xos.h>

/* Dal�� hlavi�ky */
#include <stdlib.h>
#include <stdio.h>

/* Ikona */
#include "hello_icon.h"

/*
 * Glob�ln� pou��van� deklarace
 */
char *progname; /* Jm�no programu z argv[0] */

char *display_name = NULL; /* Jmeno displeje (X serveru), NULL => vezme se
                              z environmentove promenne DISPLAY */
Display *display; /* Struktura popisuj�c� pou��van� displej */
int screen_num; /* ��slo obrazovky, kterou budeme pou��vat */
Screen *screen_ptr; /* Struktura popisuj�c� obrazovku */
unsigned int display_width, display_height; /* Rozm�ry obrazovky */

Window win; /* Top-level okno */
Pixmap icon_pixmap; /* Pixmapa pro ikonu okna */
unsigned int border_width = 4; /* ���ka okraje okna */
int x, y; /* Pozice okna na obrazovce */
unsigned int width, height; /* Rozm�ry okna */

GC gc; /* Graphics context */
XFontStruct *font_info; /* Description of font */

/*
 * Nahr�n� fontu
 */
static void load_font(XFontStruct **font_info)
{
    char *fontname = (char *) "9x15";
    
    if(!(*font_info = XLoadQueryFont(display, fontname))) {
        fprintf(stderr, "Cannot open font\n");
        exit(-1);
    }
}

/*
 * Vytvo�en� grafick�ho kontextu
 */
static void getGC(Window win, GC *gc, XFontStruct *font_info)
{
    unsigned long valuemask = 0; /* Ignorovat XGCValues a pou��t default */
    XGCValues values; /* Nastaven� grafick�ho kontextu */
    /* Atributy pro kreslen� �ar (daly by se nastavit i p�i XCreateGC) */
    unsigned int line_width = 6;
    int line_style = LineOnOffDash;
    int cap_style = CapRound;
    int join_style = JoinRound;
    int dash_offset = 0;
    static char dash_list[] = {12, 24};
    int list_length = 2;
    
    /* Vytvo�it defaultn� graphics context */
    *gc = XCreateGC(display, win, valuemask, &values);
    /* Nastavit font pro kreslen� textu */
    XSetFont(display, *gc, font_info->fid);
    /* Nastavit foreground na �ernou, proto�e default background je b�l� */
    XSetForeground(display, *gc, BlackPixel(display, screen_num));
    /* Nastavit parametry pro kreslen� �ar */
    XSetLineAttributes(display, *gc, line_width, line_style, cap_style,
                       join_style);
    XSetDashes(display, *gc, dash_offset, dash_list, list_length);
}

/*
 * Kreslen� v p��pad�, �e je okno p��li� zmen�eno
 */
static void draw_too_small(Window win, GC gc, XFontStruct *font_info)
{
    const char *small_str = "Too Small";
    int y_offset, x_offset;
    
    y_offset = font_info->ascent + 2;
    x_offset = 2;
    XDrawString(display, win, gc, x_offset, y_offset, small_str,
                (int) strlen(small_str));
}

/*
 * Kreslen� textu
 */
static void draw_text(Window win, GC gc, XFontStruct *font_info, 
                      unsigned int win_width, unsigned int win_height)
{
    const char *string1 = "Hello World!";
    const char *string2 = "To terminate program: press any key";
    const char *string3 = "or button in this window";
    const char *string4 = "Screen dimensions:";
    int len1, len2, len3, len4;
    int width1, width2, width3;
    char str_height[50], str_width[50], str_depth[50];
    int font_height;
    int initial_y_offset, x_offset;

    /* D�lky �et�zc� pro XTextWidth() a XDrawString() */
    len1 = strlen(string1);
    len2 = strlen(string2);
    len3 = strlen(string3);

    /* ���ky �et�zc� v pixelech pro centrov�n� textu */
    width1 = XTextWidth(font_info, string1, len1);
    width2 = XTextWidth(font_info, string2, len2);
    width3 = XTextWidth(font_info, string3, len3);

    /* Celkov� v��ka fontu */
    font_height = font_info->ascent + font_info->descent;

    /* Vypsat text, centrovat ��dky */
    XDrawString(display, win, gc, (int) (win_width-width1)/2,
                (int) font_height, string1, (int) len1);
    XDrawString(display, win, gc, (int) (win_width-width2)/2, 
                (int) win_height-2*font_height, string2, (int) len2);
    XDrawString(display, win, gc, (int) (win_width-width1)/2,
                (int) win_height-font_height, string1, (int) len1);

    /* Konverze ��seln�ch rozm�r� okna na �et�zce */
    sprintf(str_height, "Height - %d pixels",
            DisplayHeight(display, screen_num));
    sprintf(str_width,  "Width  - %d pixels",
            DisplayWidth(display, screen_num));
    sprintf(str_depth,  "Depth  - %d pixels",
            DefaultDepth(display, screen_num));

    /* Zjistit ���ky �et�zc� a vypsat �et�zce vertik�ln� centrovan� */
    len4 = strlen(string4);
    len1 = strlen(str_height);
    len2 = strlen(str_width);
    len3 = strlen(str_depth);

    /* M�me 4 �et�zce => horn� okraj prvn�ho bude 2*font_height nad st�edem
     * okna. V XDrawString() se zad�v� y-ov� sou�adnice baseline, kter� je
     * font_info->ascent pod horn�m okrajem textu. Proto bude offset 1. ��dku
     * od st�edu font_height+font_info->descent */
    initial_y_offset = win_height/2 - font_height - font_info->descent;
    x_offset = win_width/4;

    XDrawString(display, win, gc, x_offset, initial_y_offset, string4, len4);
    XDrawString(display, win, gc, x_offset, initial_y_offset+font_height,
                str_height, len1);
    XDrawString(display, win, gc, x_offset, initial_y_offset+2*font_height,
                str_width, len2);
    XDrawString(display, win, gc, x_offset, initial_y_offset+3*font_height,
                str_depth, len3);
}

/*
 * Kreslen� grafiky
 */
static void draw_graphics(Window win, GC gc, unsigned int win_width,
                          unsigned int win_height)
{
    int x, y;
    unsigned int width, height;

    height = win_height/2;
    width = 3*win_width/4;
    x = win_width/2 - width/2; /* Vycentrovat */
    y = win_height/2 - height/2;
    XDrawRectangle(display, win, gc, x, y, width, height);
}

/*
 * Vstupn� bod programu
 */
int main(int argc, char *argv[])
{
    XWMHints *wm_hints; /* Doporu�en� pro window manager */
    XClassHint *class_hints; /* Jm�no t��dy pro resource management */
    XTextProperty window_name, icon_name; /* Jm�na zobrazovan� window
                                               managerem */
    XSizeHints *size_hints; /* Doporu�en� velikost okna */
    char *str_window_name = (char *) "Hello World";
    char *str_icon_name = (char *) "Hello Icon";
    XEvent event; /* Pro zpracov�n� p�ijat�ch ud�lost� */
    static Bool window_too_small = False;
                        
    /* Inicializace */
    progname = argv[0];
    
    /* P�ipojen� k X serveru */
    if(!(display = XOpenDisplay(display_name))) {
        fprintf(stderr, "%s: cannot connect to X server %s\n",
                progname, XDisplayName(display_name));
        exit(-1);
    }
    printf("Connected to X server %s\n", XDisplayName(display_name));
    screen_num = DefaultScreen(display);
    printf("Default screen number is %d\n", screen_num);
    screen_ptr = DefaultScreenOfDisplay(display);

    display_width = DisplayWidth(display, screen_num);
    display_height = DisplayHeight(display, screen_num);
    printf("Display is %u x %u pixels\n", display_width, display_height);

    /* Vytvo�en� okna */
    x = y = 0; /* Okno um�st� window manager */
    width = display_width/2;
    height = display_height/2;
    win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                              x, y, width, height, border_width,
                              BlackPixel(display, screen_num),
                              WhitePixel(display, screen_num));
    printf("Created window with ID %#lx\n", win);
    
    /* Pixmapa ikony */
    icon_pixmap = XCreateBitmapFromData(display, win, hello_icon_bits,
                                        hello_icon_width, hello_icon_height);
    printf("Created pixmap with ID %#lx\n", icon_pixmap);
    
    /* Nastaven� properties pro window manager */
    if(!(size_hints = XAllocSizeHints())) {
        fprintf(stderr, "Cannot allocate memory\n");
        exit(-1);
    }
    if(!(wm_hints = XAllocWMHints())) {
        fprintf(stderr, "Cannot allocate memory\n");
        exit(-1);
    }
    if(!(class_hints = XAllocClassHint())) {
        fprintf(stderr, "Cannot allocate memory\n");
        exit(-1);
    }
    /* Poloha a velikost se bere ze skute�n�ch hodnot okna v oka�iku
     * namapov�n�. PPosition | PSize ��k�, �e hodnoty nastavil program (a
     * window manager je m��e m�nit podle pot�eby). USSize | USPosition by
     * znamenalo, �e hodnoty zadal u�ivatel (nap�. na p��kazov� ��dce) a
     * window manager by je nem�l m�nit. */
    size_hints->flags = PPosition | PSize | PMinSize;
    /* Window manager by nem�l okno nikdy zmen�it na m�n� ne�
     * min_width x min_height pixel�. */
    size_hints->min_width = 300;
    size_hints->min_height = 200;
    /* Ulo�en� jm�na okna a ikony */
    if(!XStringListToTextProperty(&str_window_name, 1, &window_name)) {
        fprintf(stderr, "XStringListToTextProperty() for window_name"
                " failed\n");
        exit(-1);
    }
    if(!XStringListToTextProperty(&str_icon_name, 1, &icon_name)) {
        fprintf(stderr, "XStringListToTextProperty() for icon_name"
                " failed\n");
        exit(-1);
    }
    wm_hints->initial_state = NormalState;/* Na za��tku m� b�t zobrazeno
                                             norm�ln� okno (ne ikona) */
    wm_hints->input = True; /* Aplikace pot�ebuje vstup z kl�vesnice */
    wm_hints->icon_pixmap = icon_pixmap;
    wm_hints->flags = StateHint | InputHint | IconPixmapHint;
    /* Jm�no programu a t��dy pro hled�n� v resource datab�zi */
    class_hints->res_name = progname;
    class_hints->res_class = (char *) "HelloWorld";
    XSetWMProperties(display, win, &window_name, &icon_name, argv, argc,
                     size_hints, wm_hints, class_hints);
    
    /* V�b�r typ� ud�lost� ke zpracov�n� */
    XSelectInput(display, win, ExposureMask | KeyPressMask |
                 ButtonPressMask | StructureNotifyMask);

    /* Nahr�t font */
    load_font(&font_info);

    /* Vytvo�it graphics context pro kreslen� */
    getGC(win, &gc, font_info);

    /* Namapovat okno na obrazovku (zobrazit) */
    XMapWindow(display, win);

    /* Smy�ka pro p��jem a zpracov�n� ud�lost� */
    while(1) {
        XNextEvent(display, &event); /* �ek� na p��chod dal�� ud�losti */
        switch(event.type) { /* Zpracov�n� ud�losti */
            case Expose:
                printf("Drawing (serial=%lu send=%d count=%d x=%d y=%d "
                       "w=%d h=%d)\n",
                       event.xexpose.serial,
                       event.xexpose.send_event,
                       event.xexpose.count,
                       event.xexpose.x,
                       event.xexpose.y,
                       event.xexpose.width,
                       event.xexpose.height);
                if(event.xexpose.count == 0) {
                    if(window_too_small) { /* P��li� zmen�en� okno */
                        printf("Drawing small\n");
                        draw_too_small(win, gc, font_info);
                    } else { /* Nakreslit obsah okna */
                        printf("Drawing normal (w=%d h=%d)\n", width, height);
                        draw_text(win, gc, font_info, width, height);
                        draw_graphics(win, gc, width, height);
                    }
                }
                break;
            case ConfigureNotify:
                /* Kdybychom nevolali XClearWindow(), tak p�i rychl� zm�n�
                 * velikosti okna my�� postupn� window manager m�n� velikost
                 * okna, co� m� za n�sledek smaz�n� p�edchoz�ho obsahu okna a
                 * vygenerov�n� ud�lost� ConfigureNotify a Expose. Jen�e
                 * jestli�e program nestihne p�ekreslit okno p�ed dal��
                 * zm�nou velikosti okna, tak X server jednou sma�e okno, pak
                 * program n�kolikr�t p�e�te ConfigureNotify a Expose a
                 * n�kolikr�t nakresl� obsah okna pro r�zn� velikosti
                 * zji�t�n� z ConfigureNotify. Takto se p�ekryje n�kolik r�zn�
                 * velk�ch kreseb a obsah okna je �patn�. */
		if(width != event.xconfigure.width || height != event.xconfigure.height)
		    XClearWindow(display, win);
                width = event.xconfigure.width;
                height = event.xconfigure.height;
                window_too_small = (int) width < size_hints->min_width+10 ||
                    (int) height < size_hints->min_height+10;
                printf("ConfigureNotify (serial=%lu send=%d w=%d h=%d)\n",
                       event.xconfigure.serial,
                       event.xconfigure.send_event,
                       event.xconfigure.width,
                       event.xconfigure.height);
                break;
            case ButtonPress:
            case KeyPress:
                /* Libovoln� kl�vesa nebo tla��tko my�i ukon�� program */
                /* N�sleduj�c� dv� vol�n� nejsou nezbytn�, XCloseDisplay()
                 * uklid� v�echny server resources alokovan� t�mto klientem */
                XUnloadFont(display, font_info->fid);
                XFreeGC(display, gc);
                /* Odpojen� od X serveru */
                XCloseDisplay(display);
                return 0;
                break;
            default:
                /* Sem se dostanou ud�losti vybran� maskou StructureNotifyMask
                 * krom� ConfigureNotify. */
                break;
        }
    }

    /* Sem se program nikdy nedostane */
    return 0;
}
