libav_link = `pkg-config --libs libavcodec libavformat libavutil libswscale`
sdl_link = -lSDL

all: libSimpleAV.a saplayer-old

libSimpleAV.a: SAMutex.o SAQueue.o SimpleAV.o
	ar cr libSimpleAV.a SAMutex.o SAQueue.o SimpleAV.o

SAMutex.o: SAMutex.h SAMutex.c
	gcc -c SAMutex.c

SAQueue.o: SAQueue.h SAQueue.c
	gcc -c SAQueue.c

### Are SAMutex.h and SAQueue.h necessary?
SimpleAV.o: SimpleAV.c SimpleAV.h SAMutex.h SAQueue.h
	gcc -c SimpleAV.c $(libav_link) $(sdl_link)

saplayer-old: saplayer-old.o libSimpleAV.a
	gcc -o saplayer-old saplayer-old.o -L. -lSimpleAV $(libav_link) $(sdl_link)

saplayer-old.o: SimpleAV.h saplayer-old.c
	gcc -c saplayer-old.c $(libav_link) $(sdl_link)

# installation
install: install-lib install-header install-player

install-lib:
	cp libSimpleAV.a /usr/local/lib

install-header:
	cp *.h /usr/local/include

install-player:
	cp saplayer-old /usr/local/bin

# un-installation
uninstall: uninstall-lib uninstall-header uninstall-player

uninstall-lib:
	rm /usr/local/lib/libSimpleAV.a

uninstall-header:
	rm /usr/local/include/SimpleAV.h /usr/local/include/SAQueue.h /usr/local/include/SAMutex.h

uninstall-player:
	rm /usr/local/bin/saplayer-old

# clean up
clean:
	rm *.o libSimpleAV.a saplayer-old