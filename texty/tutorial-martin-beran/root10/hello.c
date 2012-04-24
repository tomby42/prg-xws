/* Jednoduchý program v X s pou¾itím pouze Xlib, bez toolkitu. Ilustruje
 * základní inicializaci X klienta, vytvoøení okna, kreslení a event loop. */

/* Základní deklarace pro Xlib */
#include <X11/Xlib.h>
/* Dal¹í deklarace pro Xlib */
#include <X11/Xutil.h>
/* Pøeddefinované atomy */
#include <X11/Xatom.h>
/* Místo include pro èasto pou¾ívané systémové hlavièkové soubory */
#include <X11/Xos.h>

/* Dal¹í hlavièky */
#include <stdlib.h>
#include <stdio.h>

/* Ikona */
#include "hello_icon.h"

/*
 * Globálnì pou¾ívané deklarace
 */
char *progname; /* Jméno programu z argv[0] */

char *display_name = NULL; /* Jmeno displeje (X serveru), NULL => vezme se
                              z environmentove promenne DISPLAY */
Display *display; /* Struktura popisující pou¾ívaný displej */
int screen_num; /* Èíslo obrazovky, kterou budeme pou¾ívat */
Screen *screen_ptr; /* Struktura popisující obrazovku */
unsigned int display_width, display_height; /* Rozmìry obrazovky */

Window win; /* Top-level okno */
Pixmap icon_pixmap; /* Pixmapa pro ikonu okna */
unsigned int border_width = 4; /* ©íøka okraje okna */
int x, y; /* Pozice okna na obrazovce */
unsigned int width, height; /* Rozmìry okna */

GC gc; /* Graphics context */
XFontStruct *font_info; /* Description of font */

/*
 * Nahrání fontu
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
 * Vytvoøení grafického kontextu
 */
static void getGC(Window win, GC *gc, XFontStruct *font_info)
{
    unsigned long valuemask = 0; /* Ignorovat XGCValues a pou¾ít default */
    XGCValues values; /* Nastavení grafického kontextu */
    /* Atributy pro kreslení èar (daly by se nastavit i pøi XCreateGC) */
    unsigned int line_width = 6;
    int line_style = LineOnOffDash;
    int cap_style = CapRound;
    int join_style = JoinRound;
    int dash_offset = 0;
    static char dash_list[] = {12, 24};
    int list_length = 2;
    
    /* Vytvoøit defaultní graphics context */
    *gc = XCreateGC(display, win, valuemask, &values);
    /* Nastavit font pro kreslení textu */
    XSetFont(display, *gc, font_info->fid);
    /* Nastavit foreground na èernou, proto¾e default background je bílý */
    XSetForeground(display, *gc, BlackPixel(display, screen_num));
    /* Nastavit parametry pro kreslení èar */
    XSetLineAttributes(display, *gc, line_width, line_style, cap_style,
                       join_style);
    XSetDashes(display, *gc, dash_offset, dash_list, list_length);
}

/*
 * Kreslení v pøípadì, ¾e je okno pøíli¹ zmen¹eno
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
 * Kreslení textu
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

    /* Délky øetìzcù pro XTextWidth() a XDrawString() */
    len1 = strlen(string1);
    len2 = strlen(string2);
    len3 = strlen(string3);

    /* ©íøky øetìzcù v pixelech pro centrování textu */
    width1 = XTextWidth(font_info, string1, len1);
    width2 = XTextWidth(font_info, string2, len2);
    width3 = XTextWidth(font_info, string3, len3);

    /* Celková vý¹ka fontu */
    font_height = font_info->ascent + font_info->descent;

    /* Vypsat text, centrovat øádky */
    XDrawString(display, win, gc, (int) (win_width-width1)/2,
                (int) font_height, string1, (int) len1);
    XDrawString(display, win, gc, (int) (win_width-width2)/2, 
                (int) win_height-2*font_height, string2, (int) len2);
    XDrawString(display, win, gc, (int) (win_width-width1)/2,
                (int) win_height-font_height, string1, (int) len1);

    /* Konverze èíselných rozmìrù okna na øetìzce */
    sprintf(str_height, "Height - %d pixels",
            DisplayHeight(display, screen_num));
    sprintf(str_width,  "Width  - %d pixels",
            DisplayWidth(display, screen_num));
    sprintf(str_depth,  "Depth  - %d pixels",
            DefaultDepth(display, screen_num));

    /* Zjistit ¹íøky øetìzcù a vypsat øetìzce vertikálnì centrované */
    len4 = strlen(string4);
    len1 = strlen(str_height);
    len2 = strlen(str_width);
    len3 = strlen(str_depth);

    /* Máme 4 øetìzce => horní okraj prvního bude 2*font_height nad støedem
     * okna. V XDrawString() se zadává y-ová souøadnice baseline, která je
     * font_info->ascent pod horním okrajem textu. Proto bude offset 1. øádku
     * od støedu font_height+font_info->descent */
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
 * Kreslení grafiky
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
 * Vstupní bod programu
 */
int main(int argc, char *argv[])
{
    XWMHints *wm_hints; /* Doporuèení pro window manager */
    XClassHint *class_hints; /* Jméno tøídy pro resource management */
    XTextProperty window_name, icon_name; /* Jména zobrazovaná window
                                               managerem */
    XSizeHints *size_hints; /* Doporuèená velikost okna */
    char *str_window_name = (char *) "Hello World";
    char *str_icon_name = (char *) "Hello Icon";
    XEvent event; /* Pro zpracování pøijatých událostí */
    static Bool window_too_small = False;
                        
    /* Inicializace */
    progname = argv[0];
    
    /* Pøipojení k X serveru */
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

    /* Vytvoøení okna */
    x = y = 0; /* Okno umístí window manager */
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
    
    /* Nastavení properties pro window manager */
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
    /* Poloha a velikost se bere ze skuteèných hodnot okna v oka¾iku
     * namapování. PPosition | PSize øíká, ¾e hodnoty nastavil program (a
     * window manager je mù¾e mìnit podle potøeby). USSize | USPosition by
     * znamenalo, ¾e hodnoty zadal u¾ivatel (napø. na pøíkazové øádce) a
     * window manager by je nemìl mìnit. */
    size_hints->flags = PPosition | PSize | PMinSize;
    /* Window manager by nemìl okno nikdy zmen¹it na ménì ne¾
     * min_width x min_height pixelù. */
    size_hints->min_width = 300;
    size_hints->min_height = 200;
    /* Ulo¾ení jména okna a ikony */
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
    wm_hints->initial_state = NormalState;/* Na zaèátku má být zobrazeno
                                             normální okno (ne ikona) */
    wm_hints->input = True; /* Aplikace potøebuje vstup z klávesnice */
    wm_hints->icon_pixmap = icon_pixmap;
    wm_hints->flags = StateHint | InputHint | IconPixmapHint;
    /* Jméno programu a tøídy pro hledání v resource databázi */
    class_hints->res_name = progname;
    class_hints->res_class = (char *) "HelloWorld";
    XSetWMProperties(display, win, &window_name, &icon_name, argv, argc,
                     size_hints, wm_hints, class_hints);
    
    /* Výbìr typù událostí ke zpracování */
    XSelectInput(display, win, ExposureMask | KeyPressMask |
                 ButtonPressMask | StructureNotifyMask);

    /* Nahrát font */
    load_font(&font_info);

    /* Vytvoøit graphics context pro kreslení */
    getGC(win, &gc, font_info);

    /* Namapovat okno na obrazovku (zobrazit) */
    XMapWindow(display, win);

    /* Smyèka pro pøíjem a zpracování událostí */
    while(1) {
        XNextEvent(display, &event); /* Èeká na pøíchod dal¹í události */
        switch(event.type) { /* Zpracování události */
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
                    if(window_too_small) { /* Pøíli¹ zmen¹ené okno */
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
                /* Kdybychom nevolali XClearWindow(), tak pøi rychlé zmìnì
                 * velikosti okna my¹í postupnì window manager mìní velikost
                 * okna, co¾ má za následek smazání pøedchozího obsahu okna a
                 * vygenerování událostí ConfigureNotify a Expose. Jen¾e
                 * jestli¾e program nestihne pøekreslit okno pøed dal¹í
                 * zmìnou velikosti okna, tak X server jednou sma¾e okno, pak
                 * program nìkolikrát pøeète ConfigureNotify a Expose a
                 * nìkolikrát nakreslí obsah okna pro rùzné velikosti
                 * zji¹tìné z ConfigureNotify. Takto se pøekryje nìkolik rùznì
                 * velkých kreseb a obsah okna je ¹patný. */
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
                /* Libovolná klávesa nebo tlaèítko my¹i ukonèí program */
                /* Následující dvì volání nejsou nezbytná, XCloseDisplay()
                 * uklidí v¹echny server resources alokované tímto klientem */
                XUnloadFont(display, font_info->fid);
                XFreeGC(display, gc);
                /* Odpojení od X serveru */
                XCloseDisplay(display);
                return 0;
                break;
            default:
                /* Sem se dostanou události vybrané maskou StructureNotifyMask
                 * kromì ConfigureNotify. */
                break;
        }
    }

    /* Sem se program nikdy nedostane */
    return 0;
}
