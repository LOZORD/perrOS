#ifndef P1A2
#define P1A2

/*	You will allocate one of these structures for each key on the dial pad

This is a typedef. It creates a shorthand for a complex type specification.
In this case, you can specify the "type" KEPtr and the compiler will interpret
this as pointer to Keyboard Element.
*/

extern void PrintWrapper(void (*func)(char), char c);

struct KeyboardElement
{
	int	counter;
	char * letters;
};

typedef struct KeyboardElement* KEPtr;

#endif // P1A2
