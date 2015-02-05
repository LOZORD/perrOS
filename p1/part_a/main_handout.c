#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p1a2.h"

/* This is an array of 10 POINTERS to KeyboardElements
*/

#define NUM_KEYS 10
#define TERM_CHAR '\0'

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
KEPtr getHumanIndexedKey(int i);
char * getHumanIndexedLetterFromKey(KEPtr kP, int i);
int isValid (char * c);

//TODO test: ./tnine 223231 232181 234R

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
  char * someLetterPtr;
  KEPtr myKey;

  //iterate through the arguments ("key"words)
  for (i = 1; i < argc; i++)
  {
    //printf("in loop\n");
    keyItr = argv[i];

    while(isValid(keyItr))
    {
      keyInd = charToInt(keyItr[0]);

      myKey = getHumanIndexedKey(keyInd);

      if (myKey == NULL)
      {
        putchar('-');
        break;
      }

      // get the next char in the word
      if (keyItr[1])
      {
        letterInd = charToInt(keyItr[1]);
      }
      else
      {
        putchar('-');
        break;
        //fprintf(stderr, "even words pls");
      }

      //printf("Got keyInd: %d\tGot letterInd: %d\n", keyInd, letterInd);

      someLetterPtr = getHumanIndexedLetterFromKey(myKey, letterInd);

      //FIXME: out of range letter index
      //if (someLetterPtr == NULL || *someLetterPtr == '\0')
      if (isValid(someLetterPtr))
      {
        //fprintf(stderr,"oops\n");
        //exit(EXIT_FAILURE);
        putchar('-');
        break;
      }


      //PrintWrapper(PrintFunction, someLetter);
      //TODO print
      //printf("%c", *someLetterPtr);
      putchar(*someLetterPtr);

      myKey->counter += 1;

      // advance two chars at a time
      keyItr += 2;

    }
    //PrintWrapper(PrintFunction, '\n');
    //TODO print
    putchar('\n');
  }

  // pass our print function to PrintWrapper
  //PrintWrapper(PrintFunction, argv[1][0]);
  //int a;
  int foo;

  for (foo = 1; foo < NUM_KEYS - 1; foo++)
  {
    printf("%d\t%d\n", foo + 1, keypad[foo]->counter);
  }

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
    pad[keyItr] = (KEPtr) malloc(sizeof(struct KeyboardElement));

    if (pad[keyItr] == NULL)
    {
      fprintf(stderr, "Couldn't malloc\n");
      exit(EXIT_FAILURE);
    }

    pad[keyItr]->counter = 0;
    pad[keyItr]->letters = letters[keyItr];
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

KEPtr getHumanIndexedKey (int i)
{
  //only allow numbers human indices 2-9
  //TODO: allow human keys 1 and 10? (letterless keys?)
  if (2 <= i && i <= 9)
  {
    return keypad[i - 1];
  }
  else
  {
    return NULL;
  }
}

char * getHumanIndexedLetterFromKey(KEPtr kP, int i)
{
  //printf("\tGOT kP with letters [%s], i with val %d\n", kP->letters, i);
  char * itr;
  i -= 1;

  if (kP == NULL || i < 0)
  {
    return NULL;
  }

  itr = kP->letters;

  while (i && itr && *itr)
  {
    i--;
    itr++;
  }

  return isValid(itr) ? itr : NULL;
}

int isValid (char * ptr)
{
  return (ptr != NULL) && (*ptr != TERM_CHAR);
}
