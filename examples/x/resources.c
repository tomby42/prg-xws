/* Ukázka pou¾ití resources */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef __GNUC__
#define UNUSED
#else
#define UNUSED __attribute__((unused))
#endif

#define BUFSZ 1024

/*
 * Globálnì pou¾ívané promìnné
 */
char *display_name; /* Jméno pou¾ívaného displeje */
Display *display; /* Struktura popisující displej */
int screen_num; /* Èíslo obrazovky, kterou bude program pou¾ívat */
Screen *screen; /* Struktura popisující obrazovku */
Window root; /* Root okno */

XrmDatabase resources; /* Resource databáze */
Bool interact; /* Spustit interaktivní dotazování */

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
    
    root = RootWindow(display, screen_num);
}

/*
 * Zobrazení jedné polo¾ky databáze (voláno z XrmEnumerateDatabase())
 */
static Bool list_proc(XrmDatabase db UNUSED, XrmBindingList b, XrmQuarkList q,
		      XrmRepresentation *type UNUSED, XrmValue *value,
		      XPointer arg UNUSED)
{
    while(*q != NULLQUARK) {
	if(*b++ == XrmBindTightly)
	    printf(".");
	else
	    printf("*");
	printf(XrmQuarkToString(*q));
	q++;
    }
    printf(": %s\n", (char *)value->addr);
    return False;
}

/*
 * Pøeèíst konfiguraci (vytvoøit resource databázi)
 */
static void create_resources(int argc, char *argv[])
{
    char *type, *str;
    XrmValue val;
    XrmDatabase db_cmdline = NULL, db_xrm = NULL, db_file = NULL;
    XrmDatabase db_display = NULL, db_screen = NULL;
    static XrmOptionDescRec table[] ={
	{ (char *)"-rm", (char *)".resourceManager", XrmoptionNoArg,
	    (caddr_t)"Yes" },
	{ (char *)"-sr", (char *)".screenResources", XrmoptionNoArg,
	    (caddr_t)"Yes" },
	{ (char *)"-file", (char *)".file", XrmoptionSepArg, (caddr_t)NULL },
	{ (char *)"-xrm", (char *)".xrm", XrmoptionSepArg, (caddr_t)NULL },
	{ (char *)"-list", (char *)".list", XrmoptionNoArg, (caddr_t)"Yes" },
	{ (char *)"-interact", (char *)".interact", XrmoptionNoArg,
	    (caddr_t)"Yes" },
    };
    XrmQuark nullquark = NULLQUARK;
    
    XrmInitialize();

    debug("Parsing command line\n");
    XrmParseCommand(&db_cmdline, table, sizeof(table)/sizeof(table[0]),
		    "resources", &argc, argv);
    if(argc != 1) {
	fprintf(stderr, "usage: resources [-rm] [-sr] [-file name] "
		"[-xrm string] [-list] [-interact]\n");
	exit(1);
    }

    if(XrmGetResource(db_cmdline, "resources.resourceManager",
		      "Resources.ResourceManager", &type, &val) &&
       (str = XResourceManagerString(display))) {
	debug("Parsing resources from RESOURCE_MANAGER\n");
	db_display = XrmGetStringDatabase(str);
    }

    if(XrmGetResource(db_cmdline, "resources.screenResources",
		      "Resources.ScreenResources", &type, &val) &&
       (str = XScreenResourceString(screen))) {
	debug("Parsing resources from SCREEN_RESOURCES\n");
	db_screen = XrmGetStringDatabase(str);
    }
   
    if(XrmGetResource(db_cmdline, "resources.file", "Resources.File", &type,
		      &val)) {
	debug("Parsing resources from file \"%s\"\n", (char *)val.addr);
	if(!(db_file = XrmGetFileDatabase((char *)val.addr)))
	    fprintf(stderr, "Cannot read file \"%s\"\n", (char *)val.addr);
    }

    if(XrmGetResource(db_cmdline, "resources.xrm", "Resources.Xrm", &type,
		      &val)) {
	debug("Parsing resources from argument \"-xrm\"\n");
	db_xrm = XrmGetStringDatabase((char *)val.addr);
    }

    debug("Combining databases\n");
    XrmMergeDatabases(db_display, &resources);
    XrmMergeDatabases(db_screen, &resources);
    XrmMergeDatabases(db_file, &resources);
    XrmMergeDatabases(db_xrm, &resources);

    if(XrmGetResource(db_cmdline, "resources.list", "Resources.List", &type,
		      &val)) {
	debug("Contents of resource database:\n");
	XrmEnumerateDatabase(resources, &nullquark, &nullquark,
			     XrmEnumAllLevels, list_proc, NULL);
    }
    
    interact = XrmGetResource(db_cmdline, "resources.interact",
			      "Resources.Interact", &type, &val);
	
    XrmDestroyDatabase(db_cmdline);
    
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
    char buf[BUFSZ], bufc[BUFSZ];
    size_t l;
    char *type;
    XrmValue val;
    
    open_display();
    create_resources(argc, argv);

    if(interact) {
	fprintf(stderr, "Name: ");
	fflush(stderr);
	while(fgets(buf, BUFSZ, stdin)) {
	    l = strlen(buf);
	    if(l > 0 && buf[l-1] == '\n')
		buf[l-1] = '\0';
	    fprintf(stderr, "Class: ");
	    fflush(stderr);
	    fgets(bufc, BUFSZ, stdin);
	    l = strlen(bufc);
	    if(l > 0 && bufc[l-1] == '\n')
		bufc[l-1] = '\0';
	    printf("(%s, %s) -> ", buf, bufc);
	    if(XrmGetResource(resources, buf, bufc, &type, &val))
		printf("\"%s\"\n", (char *)val.addr);
	    else
		printf("None\n");
	    fprintf(stderr, "Name: ");
	    fflush(stderr);
	}
    }
    
    close_display();
   
    return 0;
}
