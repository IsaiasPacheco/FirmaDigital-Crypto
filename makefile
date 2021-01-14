PROYECTO=firmadigital
DESTDIR=/usr/local/bin
PKGCONFIG = $(shell which pkg-config)
OBJECTS=$(patsubst %.c,%.o,$(wildcard *.c))
CFLAGS=-Wall $(shell $(PKGCONFIG) --cflags gtk+-3.0)
LDFLAGS=-lm
LIBS = $(shell $(PKGCONFIG) --libs gtk+-3.0) -lcrypto
CC=gcc -lcrypto


all:	$(PROYECTO)

%.o:	%.c
	$(CC) -c *.c $(CFLAGS)

$(PROYECTO):  $(OBJECTS)
	$(CC) -o $@ $(LIBS) $(OBJECTS) $(LDFLAGS)	

.PHONY:	clean

install:	all
	if	[ ! -d $(DESTDIR) ]; then \
		sudo mkdir $(DESTDIR); \
	fi; \
	sudo cp $(PROYECTO) $(DESTDIR)

uninstall:
	sudo rm $(DESTDIR)/$(PROYECTO)

clean:
	rm -f *.o $(PROYECTO)	