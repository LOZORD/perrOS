#include <stdlib.h>
#include <stdio.h>

#define MAX   10
#define DEBUG  0

typedef struct table
{
  int * entries;
  int dimension;
} Table;

int * at (Table * t, int x, int y);
void fill (Table * t);
void printTable (Table * t);

int main (int argc, char ** argv)
{
  int dim;
  int temp;
  Table table;

  if (argc < 2)
  {
    dim = MAX;
  }
  else
  {
    temp = atoi(*(argv + 1));

    if (temp > MAX)
    {
      dim = MAX;
    }
    else if (temp < 1)
    {
      dim = MAX;
    }
    else
    {
      dim = temp;
    }
  }

  #if DEBUG
  printf("Using dimension %d\n", dim);
  #endif

  table.dimension = dim;

  //TODO check return val
  table.entries = malloc(sizeof(int) * dim * dim);

  #if DEBUG
  printf("Filling table\n");
  #endif

  fill(&table);

  #if DEBUG
  printf("Printing table\n");
  #endif

  printTable(&table);

  #if DEBUG
  printf("Freeing entries\n");
  #endif

  free(table.entries);

  #if DEBUG
  printf("Done\n\n");
  #endif

  return EXIT_SUCCESS;
}

void fill (Table * table)
{
  int i, j, val;
  int * loc;

  for (i = 0; i < table->dimension; i++)
  {
    for (j = 0; j < table->dimension; j++)
    {
      val = (i + 1) * (j + 1);
      #if DEBUG
      printf("i is %d, j is %d", i, j);
      printf("\tval is %d\n", val);
      #endif

      loc = at(table, i , j);

      #if DEBUG
      printf("loc is %p\n", loc);
      #endif

      (*loc) = (i + 1) * (j + 1);
    }
  }
}

void printTable (Table * table)
{
  int i, j, val;

  for (i = 0; i < table->dimension; i++)
  {
    for (j = 0; j < table->dimension; j++)
    {
      val = *(at(table, i, j));
      printf("%d ", val);
    }
    printf("\n");
  }
}

int * at (Table * table, int i, int j)
{
  //TODO refactor
  //int * ets = (void *)table->entries;
  //int d = table->dimension;
  //return (ets+j) * (d + i);
  //TODO refactor 8|
  return table->entries + ((i * table->dimension) + j); //returns the pointer
}
