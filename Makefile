libav_link = -pthread -L/usr/local/lib -lavformat -lavcodec -ldl -lasound -lz -lswscale -lavutil -lm
sdl_link = -lSDL -lSDL

all: saplayer-old

saplayer-old: SAMutex.o SAQueue.o SimpleAV.o saplayer-old.o
	gcc -o saplayer-old SAMutex.o SAQueue.o SimpleAV.o saplayer-old.o $(libav_link) $(sdl_link)

SAMutex.o: SAMutex.h SAMutex.c
	gcc -c SAMutex.c

SAQueue.o: SAQueue.h SAQueue.c
	gcc -c SAQueue.c

# Are SAMutex.h and SAQueue.h necessary?
SimpleAV.o: SimpleAV.c SimpleAV.h SAMutex.h SAQueue.h
	gcc -c SimpleAV.c $(libav_link) $(sdl_link)

saplayer-old.o: SimpleAV.h saplayer-old.c
	gcc -c saplayer-old.c $(libav_link) $(sdl_link)