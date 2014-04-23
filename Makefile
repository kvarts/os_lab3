terminal: main.o
	gcc -o terminal main.o 
main.o:	main.c
	gcc -c main.c
clean:
	rm *.o terminal
