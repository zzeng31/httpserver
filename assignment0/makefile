default: shoulders
shoulders.o: shoulders.c
	gcc -c shoulders.c -o shoulders.o
shoulders: shoulders.o
	gcc shoulders.o -o shoulders

clean:
	-rm -f shoulders.o
	-rm -f shoulders