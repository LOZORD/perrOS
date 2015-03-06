all: mem.c test_mem.c lmem
	gcc -lmem -L. -o test_mem test_mem.c -Wall -Werror

lmem:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o

clean:
	rm test_mem
	rm libmem.so
	rm mem.o
