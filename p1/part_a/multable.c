#include <stdlib.h>
#include <stdio.h>

#define DEBUG  0 // whether we want to print debug info or no
#define MAX   10 // maximum value for our table's dimensions

typedef struct table
{
  int * entries;
  int dimension; // our table is a square (n x n)
} Table;

void initTable (Table * t, int d);
void destroyTable (Table * t);
int * at (Table * t, int x, int y);
void fill (Table * t);
void printTable (Table * t);

int main (int argc, char ** argv)
{
  int dim;
  Table table;

  // if no integer argument is supplied, then just use MAX
  if (argc < 2)
  {
    dim = MAX;
  }
  else
  {
    dim = atoi(*(argv + 1));
  }

  #if DEBUG
  printf("Initializing the table\n");
  #endif

  initTable(&table, dim);

  #if DEBUG
  printf("Filling table\n");
  #endif

  fill(&table);

  #if DEBUG
  printf("Printing table\n");
  #endif

  printTable(&table);

  #if DEBUG
  printf("Destroying the table\n");
  #endif

  destroyTable(&table);

  #if DEBUG
  printf("Done\n\n");
  #endif

  return EXIT_SUCCESS;
}

/* initTable (Table * t, int d)
 * Initializes the fields of t so that
 * t's dimension is d if d is valid, else MAX
 * t's entries is a grid (implemented as a contiguous 1d array)
 *   with size (dimension x dimension)
 */
void initTable (Table * table, int dim)
{
  // any out of bounds dim argument gets set to MAX
  if (dim > MAX)
  {
    dim = MAX;
  }
  else if (dim < 1)
  {
    dim = MAX;
  }
  else
  {
    // dim stays the same
  }

  #if DEBUG
  printf("Using dimension %d\n", dim);
  #endif

  table->dimension = dim;

  table->entries = (int *) malloc(sizeof(int) * dim * dim);

  if (table->entries == NULL)
  {
    fprintf(stderr, "ERROR: Could not allocate memory for multable\n");
    exit(EXIT_FAILURE);
  }
}

/* destroyTable (Table * t)
 * Destroys t by removing it's dimension value and freeing its entries array
 */
void destroyTable (Table * table)
{
  // remove the dimension property
  table->dimension = 0;
  // free the dynamically allocated entries
  free(table->entries);
}

/* fill (Table * t)
 * Fills the t's entries by inserting products at their correct locations
 */
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

/* printTable (Table * t)
 * Prints out the contents of t's entries, as a 2d grid,
 * row by row. The printing uses tabs and goes to stdout.
 */
void printTable (Table * table)
{
  int i, j, val;

  for (i = 0; i < table->dimension; i++)
  {
    for (j = 0; j < table->dimension; j++)
    {
      val = *(at(table, i, j));
      printf("%d\t", val);
    }
    printf("\n");
  }
}

/* at (Table * t, int i, int j)
 * Provides an abstraction of a 2d grid for t's 1d entry array.
 *    TODO check the math below...
 * If t->entries was a 2d array, this function would be equivalent to &(t[i][j]).
 * The following calculation is used to return a pointer to the int at t[i][j].
 * t->entries + (j * t->dim) + i (a simulated 2d grid)
 *
 * This function checks whether the given i and j are valid,
 * when DEBUG is true (1).
 */
int * at (Table * table, int i, int j)
{
  #if DEBUG
  if (i < 0 ||
      j < 0 ||
      i >= table->dimension ||
      j >= table->dimension)
  {
    fprintf("`at` received bad values\ti:%d\tj:%d\n", i, j);
    exit(EXIT_FAILURE);
  }
  #endif

  return table->entries + (j * table->dimension) + i;
}
