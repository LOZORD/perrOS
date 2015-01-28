#include <stdlib.h>
#include <stdio.h>

#define MAX 10

typedef struct table
{
  int * entries;
  int dimension;
} Table;

int * at (Table * t, int x, int y);
void fill (Table * t);
void printTable (Table * t);

int DEBUG = 1;

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

  DEBUG &&= printf("Using dimension %d\n", dim);

  table.dimension = dim;

  //TODO check return val
  table.entries = malloc(sizeof(int) * dim * dim);

  DEBUG &&= printf("Filling table\n");

  fill(&table);

  DEBUG &&= printf("Printing table\n");

  printTable(&table);

  DEBUG &&= printf("Freeing entries\n");

  free(table.entries);

  DEBUG &&= printf("Done\n\n");

  return EXIT_SUCCESS;
}

void fill (Table * table)
{
  int i, j;
  int * temp;

  for (i = 1; i <= table->dimension; i++)
  {
    for (j = 1; j <= table->dimension; j++)
    {
      DEBUG &&= printf("i is %d, j is %d\n", i, j);
      temp = at(table, i - 1, j - 1);
      DEBUG &&= printf("temp is %p\n", temp);
      (*temp) = i * j;
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
  //int * ets = (void *)table->entries;
  //int d = table->dimension;
  //return (ets+j) * (d + i);
  //TODO refactor 8|
  return table->entries + ((i * table->dimension) + j); //returns the pointer
}
