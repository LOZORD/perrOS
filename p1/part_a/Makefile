all: multable tnine

multable: multable.c
	gcc -Wall multable.c -o multable

tnine: p1a2.h printwrapper.o main_handout.c
	gcc -Wall p1a2.h printwrapper.o main_handout.c -o tnine

clean:
	rm multable tnine
