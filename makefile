all: useful.c
	rm -f sock_test
	gcc -c useful.c -lm -o useful.o
	gcc useful.o server.c  -o server.a
	gcc useful.o client.c -o client.a

clean:
	rm -f *.o