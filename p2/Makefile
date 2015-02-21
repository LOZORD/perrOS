all: mysh.c mytee.c
	gcc -Wall -Werror mysh.c -O3 -o mysh
	gcc -Wall -Werror mytee.c -O3 -o mytee

clean: 
	rm mysh
	rm mytee

val: mysh
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./mysh

pytest: mysh mytee
	python ~cs537-1/testing/p2a/ShellTest.py .
