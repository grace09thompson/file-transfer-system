default: ftclient

ftclient.o: ftclient.c ftclient.h
	gcc -Wall -std=gnu99 -c ftclient.c

ftclient: ftclient.o
	gcc -Wall -std=gnu99 -o ftclient ftclient.o main.c

clean:
	rm ftclient.o

cleanall: clean
	rm ftclient
