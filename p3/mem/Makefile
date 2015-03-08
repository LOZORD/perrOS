all: mem.c test_mem.c lmem
	gcc -lmem -L. -o run_test_mem test_mem.c -Wall -Werror

lmem:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o

clean:
	rm run_test_mem mem.o libmem.so

setenv:
	#export LD_LIBRARY_PATH=[DOLLAR_SIGN]PWD
	#echo [DOLLAR_SIGN]LD_LIBRARY_PATH
	echo "run these commands above"
