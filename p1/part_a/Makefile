all: multable tnine

multable: multable.c
	gcc -Wall multable.c -o multable

tnine: p1a2.h printwrapper.o main_handout.c
	gcc -Wall p1a2.h printwrapper.o main_handout.c -o tnine

clean:
	rm multable tnine

VAR = ""

val-m: multable
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./multable $(VAR)
val-t: tnine
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./tnine $(VAR)
