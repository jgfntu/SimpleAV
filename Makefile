CFLAGS=-g -Wall -O2 `pkg-config --cflags libavcodec libavformat libavutil libswscale sdl`
LIBS=`pkg-config --libs libavcodec libavformat libavutil libswscale sdl`

all: libSimpleAV.a saplayer-old

libSimpleAV.a: SAMutex.o SAQueue.o SimpleAV.o
	ar cr libSimpleAV.a SAMutex.o SAQueue.o SimpleAV.o

SAMutex.o: SAMutex.h SAMutex.c
	gcc $(CFLAGS) -c SAMutex.c

SAQueue.o: SAQueue.h SAQueue.c
	gcc $(CFLAGS) -c SAQueue.c

SimpleAV.o: SimpleAV.c SimpleAV.h SAMutex.h SAQueue.h
	gcc $(CFLAGS) -c SimpleAV.c

saplayer-old: saplayer-old.o libSimpleAV.a
	gcc $(LIBS) -o saplayer-old saplayer-old.o libSimpleAV.a

saplayer-old.o: SimpleAV.h saplayer-old.c
	gcc $(CFLAGS) -c saplayer-old.c

### install & uninstall & clean up

install:
	mkdir -p /usr/local/lib /usr/local/include /usr/local/bin
	cp libSimpleAV.a /usr/local/lib
	cp SimpleAV.h /usr/local/include
	cp saplayer-old /usr/local/bin

uninstall:
	rm /usr/local/lib/libSimpleAV.a
	rm /usr/local/include/SimpleAV.h
	rm /usr/local/bin/saplayer-old

clean:
	rm *.o libSimpleAV.a saplayer-old
