test: test.o SAQueue.o SA_api.o
	gcc -g -Wall -o test test.o SAQueue.o SA_api.o `pkg-config --static --libs libavcodec libavformat libswscale` -lSDL 

test.o: test.c SA_api.h
	gcc -g -Wall -c test.c

SA_api.o: SA_api.c SA_api.h SAQueue.h
	gcc -g -Wall -c SA_api.c

SAQueue.o: SAQueue.c SAQueue.h
	gcc -g -Wall -c SAQueue.c

clean:
	rm test.o SAQueue.o SA_api.o test