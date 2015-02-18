all: mysh.c
	gcc -Wall -Werror mysh.c -O3 -o run_mysh

clean: run_mysh
	rm run_mysh

val: run_mysh
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./run_mysh
