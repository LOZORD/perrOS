#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p1a2.h"

/* This is an array of 10 POINTERS to KeyboardElements
*/

#define NUM_KEYS 10

//array of pointers!
KEPtr keypad[NUM_KEYS];
char * letters[NUM_KEYS] = { "", "ABC", "DEF", "GHI", "JKL", "MNO", "PQRS", "TUV", "WXYZ", "" };

void initKeypad (KEPtr ptrArr [NUM_KEYS]);
void destroyKeypad (KEPtr ptrArr [NUM_KEYS]);
void padPrinter (char c);
// our dummy printer function (used as a tester)
void PrintFunction (char c);
int charToInt (char c);
int charPtrToInt (char * c);

/*
 * A very janky solution
char * globalItr = NULL;

char iterateThroughStr (char * str)
{
  return str[globalItr++];
}
*/

int main(int argc, char * argv[])
{
  if (argc <= 1)
  {
    printf("Usage: p1a2 string1 [stringN]\n");
    exit(1);
  }

  // YOUR CODE HERE
  initKeypad(keypad);

  int keyInd, letterInd, i;
  char * keyItr;
  char someLetter;

  //iterate through the arguments ("key"words)
  for (i = 1; i < argc; i++)
  {
    keyItr = argv[i];

    while(keyItr)
    {
      keyInd = charToInt(keyItr[0]);

      //TODO keyInd in bounds?

      if (keyInd < 2 || keyInd > 9)
      {
        //TODO see updated spec
        fprintf(stderr, "bad input");
        break;
      }

      // get the next char in the word
      if (keyItr[1])
      {
        letterInd = charPtrToInt(keyItr[1]);
      }
      else
      {
        fprintf(stderr, "even words pls");
      }

      someLetter = letters[keyInd][letterInd];
      //PrintWrapper(PrintFunction, someLetter);
      //TODO print
      fprintf(stderr,"%c", someLetter);

      keypad[keyInd]->counter += 1;
      //the letters that this key "contains"
      keypad[keyInd]->letters = letters[i];

      // advance two chars at a time
      keyItr += 2;

    }
    //PrintWrapper(PrintFunction, '\n');
    //TODO print
    fprintf(stderr,"\n");
  }

  // TODO

  // pass our print function to PrintWrapper
  PrintWrapper(PrintFunction, argv[1][0]);

  destroyKeypad(keypad);
  return 0;
}

void PrintFunction(char c)
{
  printf("%c\n", c);
}

/* alternative: do pad = malloc(NUM_KEYS * sizeof(struct KeyboardElement)) */
void initKeypad (KEPtr pad [NUM_KEYS])
{
  int keyItr;
  for (keyItr = 0; keyItr < NUM_KEYS; keyItr++)
  {
    //FIXME what kind of data structure do we want to malloc?
    pad[keyItr] = (KEPtr) malloc(sizeof(struct KeyboardElement));

    if (pad[keyItr] == NULL)
    {
      fprintf(stderr, "Couldn't malloc\n");
      exit(EXIT_FAILURE);
    }

    pad[keyItr]->counter = 0;
    pad[keyItr]->letters = NULL;
  }
}

void destroyKeypad (KEPtr pad [NUM_KEYS])
{
  // free all of the pad's key elements
  int keyItr;
  for (keyItr = 0; keyItr < NUM_KEYS; keyItr++)
  {
    free(pad[keyItr]);
  }
}

int charToInt (char c)
{
  return (int)(c - '0');
}

int charPtrToInt (char * c)
{
  return charToInt(*c);
}
