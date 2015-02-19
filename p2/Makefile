all: mysh.c
	gcc -Wall -Werror mysh.c -O3 -o mysh

clean: mysh
	rm mysh

val: mysh
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./mysh

pytest: mysh
	python ~cs537-1/testing/p2a/ShellTest.py .
