CC= gcc
CFLAGS= -g -Wall -W -Wpointer-arith -Wbad-function-cast \
	-Wwrite-strings -Wconversion -Wstrict-prototypes -Wmissing-prototypes \
	-I/usr/X11R6/include
LDFLAGS= -L/usr/X11R6/lib
LIB= -lX11

TARGETS= hello hello2 background expose gravity cursor \
line_style cap_join fill_rule arc_mode clipping fill_style \
copy_area colormap grab i18n selection resources

SOURCES= hello.c hello2.c background.c expose.c gravity.c cursor.c \
line_style.c cap_join.c fill_rule.c arc_mode.c clipping.c fill_style.c \
copy_area.c colormap.c grab.c i18n.c selection.c resources.c

.SUFFIXES:
.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(TARGETS)

clean:
	rm -f *.o

cleanbin:
	rm -f $(TARGETS)

tags: $(SRCS)
	ctags --c-types=+p+x

depend:
	makedepend $(SOURCES)

list :
	for f in *.c;do echo $$f;awk 'NR==1,/\*\//{print}' $$f;echo;done

hello: hello.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

hello2: hello2.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

expose: expose.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

background: background.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

gravity: gravity.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

cursor: cursor.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

line_style: line_style.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

cap_join: cap_join.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

fill_rule: fill_rule.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

arc_mode: arc_mode.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

clipping: clipping.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

fill_style: fill_style.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

copy_area: copy_area.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

colormap: colormap.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

grab: grab.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

i18n: i18n.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

selection: selection.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

resources: resources.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB)

# DO NOT DELETE

hello.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
hello.o:  /usr/include/stdio.h hello_icon.h
hello2.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
hello2.o:  /usr/include/stdio.h
hello2.o:  Xlib_icon.h
background.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
background.o:  /usr/include/stdio.h
background.o:  Xlib_icon.h pattern1.h pattern2.h
expose.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
expose.o:  /usr/include/stdio.h
expose.o:  Xlib_icon.h
gravity.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
gravity.o: /usr/include/stdio.h
gravity.o:  Xlib_icon.h
cursor.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
cursor.o:  /usr/include/stdio.h
cursor.o:  Xlib_icon.h cursor_source.h cursor_mask.h
line_style.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
line_style.o: /usr/include/stdio.h
line_style.o: Xlib_icon.h
cap_join.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
cap_join.o:  /usr/include/stdio.h
cap_join.o:  Xlib_icon.h
fill_rule.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
fill_rule.o: /usr/include/stdio.h
fill_rule.o:  Xlib_icon.h
arc_mode.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
arc_mode.o: /usr/include/stdio.h
arc_mode.o: Xlib_icon.h
clipping.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
clipping.o:  /usr/include/stdio.h
clipping.o:  Xlib_icon.h
fill_style.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
fill_style.o: /usr/include/stdio.h
fill_style.o:  Xlib_icon.h pattern2.h
copy_area.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
copy_area.o: /usr/include/stdio.h
copy_area.o: Xlib_icon.h pattern2.h
colormap.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
colormap.o: /usr/include/stdio.h
colormap.o:  /usr/include/sys/time.h
colormap.o: /usr/include/sys/types.h
colormap.o: /usr/include/time.h
colormap.o: /usr/include/unistd.h Xlib_icon.h
grab.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
grab.o: /usr/include/stdio.h
grab.o:  Xlib_icon.h
i18n.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
i18n.o: /usr/include/stdio.h
i18n.o: /usr/include/locale.h /usr/include/wchar.h
i18n.o: /usr/include/wctype.h Xlib_icon.h
selection.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
selection.o: /usr/include/stdio.h
selection.o: /usr/include/limits.h
selection.o:
selection.o:  Xlib_icon.h
resources.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
resources.o: /usr/include/stdio.h
resources.o: /usr/include/ctype.h
resources.o: 
