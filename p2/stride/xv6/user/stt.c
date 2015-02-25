#include "types.h"
#include "stat.h"
#include "user.h"
int main (int argc, char ** argv)
{
  int rc;
  printf(1,"Hello world!\n\n");
  procdump();
  rc = settickets(150);
  procdump();
  printf(1, "FOR SETTICKETS 150, GOT RC OF %d\n", rc);

  rc = settickets(10);
  procdump();
  printf(1, "FOR SETTICKETS 10, GOT RC OF %d\n", rc);
  
  rc = settickets(160);
  procdump();
  printf(1, "FOR SETTICKETS 160, GOT RC OF %d\n", rc);
  
  rc = settickets(0);
  procdump();
  printf(1, "FOR SETTICKETS 0, GOT RC OF %d\n", rc);
  
  rc = settickets(11);
  procdump();
  printf(1, "FOR SETTICKETS 11, GOT RC OF %d\n", rc);

  exit();
}
