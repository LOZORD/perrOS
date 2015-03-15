all: mem.c test_mem.c lmem
	gcc -lmem -L. -o run_test_mem test_mem.c -Wall -Werror
	gcc -lmem -L. -o verif_nextfit verify_nextfit.c -Wall -Werror
	gcc -lmem -L. -pthread -o threaded threaded.c -Wall -Werror

lmem:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o

clean:
	rm run_test_mem verif_nextfit mem.o libmem.so threaded

setenv:
	#run the command below
	export LD_LIBRARY_PATH='$(shell pwd)'
pytest:
	python /u/c/s/cs537-1/testing/p3a/MemTest.py .
