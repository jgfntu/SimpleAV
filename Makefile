LIBAV_FLAGS = `pkg-config --static --libs libavcodec libavformat libswscale`

saplayer: saplayer.o SAQueue.o SA_api.o
	gcc -o saplayer saplayer.o SAQueue.o SA_api.o $(LIBAV_FLAGS) -lSDL 

saplayer.o: saplayer.c SA_api.h
	gcc -O3 -c saplayer.c

SA_api.o: SA_api.c SA_api.h SAQueue.h
	gcc -O3 -c SA_api.c

SAQueue.o: SAQueue.c SAQueue.h
	gcc -O3 -c SAQueue.c

debug: saplayer_g.o SAQueue_g.o SA_api_g.o
	gcc -g -Wall -o debug saplayer_g.o SAQueue_g.o SA_api_g.o $(LIBAV_FLAGS) -lSDL

saplayer_g.o: saplayer.c SA_api.h
	gcc -g -Wall -o saplayer_g.o -c saplayer.c

SA_api_g.o: SA_api.c SA_api.h SAQueue.h
	gcc -g -Wall -o SA_api_g.o -c SA_api.c

SAQueue_g.o: SAQueue.c SAQueue.h
	gcc -g -Wall -o SAQueue_g.o -c SAQueue.c

clean:
	rm *.o
