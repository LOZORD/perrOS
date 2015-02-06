/***** TNINE PROGRAM *****
 * Written by Leo Rudberg in 2015
 * P1A2 for CS 537 : Operating Systems
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p1a2.h"

#define NUM_KEYS 10
#define TERM_CHAR '\0'
#define DEBUG 0

/* This is an array of 10 POINTERS to KeyboardElements */
KEPtr keypad[NUM_KEYS];
/* Letters that are associated with each key */
char * letters[NUM_KEYS] = { "", "ABC", "DEF", "GHI", "JKL", "MNO", "PQRS", "TUV", "WXYZ", "" };

/* TODO comment */
void initKeypad (KEPtr ptrArr [NUM_KEYS]);
void destroyKeypad (KEPtr ptrArr [NUM_KEYS]);
void padPrinter (char c);
// our dummy printer function (used as a tester)
int charToInt (char c);
KEPtr getHumanIndexedKey(int i);
char * getHumanIndexedLetterFromKey(KEPtr kP, int i);
int isValid (char * c);
void printErrorDash ();
void printCharWithWrapper (char c);
void printStringWithWrapper (char * c);
char * writeDecimalIntToBuffer(int i, char * c);

//TODO test: ./tnine 223231 232181 234R

int main(int argc, char * argv[])
{
  if (argc <= 1)
  {
    fprintf(stderr, "Usage: p1a2 string1 [stringN]\n");
    exit(EXIT_FAILURE);
  }

  initKeypad(keypad);

  int keyInd, letterInd, i;
  char * keyItr;
  char * someLetterPtr;
  KEPtr myKey;

  //iterate through the arguments ("key"words)
  for (i = 1; i < argc; i++)
  {
    keyItr = argv[i];

    while(isValid(keyItr))
    {
      keyInd = charToInt(keyItr[0]);

      myKey = getHumanIndexedKey(keyInd);

      if (myKey == NULL)
      {
        printErrorDash();
        break;
      }

      // get the next char in the word
      if (isValid(keyItr + 1))
      {
        letterInd = charToInt(keyItr[1]);
      }
      else
      {
        printErrorDash();
        break;
      }

      #if DEBUG
      printf("Got keyInd: %d\tGot letterInd: %d\n", keyInd, letterInd);
      #endif

      someLetterPtr = getHumanIndexedLetterFromKey(myKey, letterInd);

      if (!isValid(someLetterPtr))
      {
        printErrorDash();
        break;
      }

      printCharWithWrapper(*someLetterPtr);

      myKey->counter += 1;

      // advance two chars at a time
      keyItr += 2;
    }

    printCharWithWrapper('\n');
  }

  //for writing strings of integers into
  //integers should not be larger than about 10 chars in length
  char buff [33];

  #if DEBUG
  printf("Printing out counts now\n");
  #endif

  for (i = 1; i < NUM_KEYS - 1; i++)
  {
    #if DEBUG
    printf("i is %d\n",i);
    #endif
    writeDecimalIntToBuffer(i + 1, buff);
    printStringWithWrapper(buff);
    printCharWithWrapper('\t');
    writeDecimalIntToBuffer(keypad[i]->counter, buff);
    printStringWithWrapper(buff);
    printCharWithWrapper('\n');
  }

  destroyKeypad(keypad);

  return EXIT_SUCCESS;
}

//What we will pass to PrintWrapper
void PrintChar (char c)
{
  putchar(c);
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
      fprintf(stderr, "Unsuccessful malloc\n");
      exit(EXIT_FAILURE);
    }

    pad[keyItr]->counter = 0;
    pad[keyItr]->letters = letters[keyItr];
  }
}

//free all of the pad's key elements
void destroyKeypad (KEPtr pad [NUM_KEYS])
{
  int keyItr;

  for (keyItr = 0; keyItr < NUM_KEYS; keyItr++)
  {
    free(pad[keyItr]);
  }
}

//force-converts a character to an int using ASCII arithmetic
int charToInt (char c)
{
  return (int)(c - '0');
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
  #if DEBUG
  printf("\tGOT kP with letters [%s], i with val %d\n", kP->letters, i);
  #endif

  char * itr;

  i -= 1;

  if (kP == NULL || i < 0)
  {
    return NULL;
  }

  itr = kP->letters;

  while (i != 0 && isValid(itr))
  {
    i--;
    itr++;
  }

  return isValid(itr) ? itr : NULL;
}

//test whether a character pointer points to valid data
int isValid (char * ptr)
{
  return (ptr != NULL) && (*ptr != TERM_CHAR);
}

//uses the PrintWrapper
void printCharWithWrapper (char c)
{
  PrintWrapper(PrintChar, c);
}

//for printing errors
void printErrorDash ()
{
  printCharWithWrapper('-');
}

void printStringWithWrapper (char * c)
{
  char * itr = c;

  while(isValid(itr))
  {
    printCharWithWrapper(*itr);
    itr++;
  }
}

char * writeDecimalIntToBuffer (int i, char * buff)
{
  if (sprintf(buff, "%d", i) <= 0)
  {
    fprintf(stderr, "Buffer error");
    exit(EXIT_FAILURE);
  }
  else
  {
    return buff;
  }
}
