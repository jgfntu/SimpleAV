test: test.o SAQueue.o SA_api.o
	gcc -o test test.o SAQueue.o SA_api.o `pkg-config --static --libs libavcodec libavformat libswscale` -lSDL 

test.o: test.c SA_api.h
	gcc -O3 -c test.c

SA_api.o: SA_api.c SA_api.h SAQueue.h
	gcc -O3 -c SA_api.c

SAQueue.o: SAQueue.c SAQueue.h
	gcc -O3 -c SAQueue.c

debug: test_g.o SAQueue_g.o SA_api_g.o
	gcc -g -Wall -o debug test_g.o SAQueue_g.o SA_api_g.o `pkg-config --static --libs libavcodec libavformat libswscale` -lSDL 

test_g.o: test.c SA_api.h
	gcc -g -Wall -o test_g.o -c test.c

SA_api_g.o: SA_api.c SA_api.h SAQueue.h
	gcc -g -Wall -o SA_api_g.o -c SA_api.c

SAQueue_g.o: SAQueue.c SAQueue.h
	gcc -g -Wall -o SAQueue_g.o -c SAQueue.c

clean:
	rm *.o
