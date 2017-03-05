

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"

const char* logPrefix = "";

int main()
{

   Sem s(0x3db00001);

   if (s.check() == success)
      printf("free \n");
   else
      printf("locked \n");

   printf("inc\n");

   s.inc();

   printf("inc done\n");
   sleep(5);

   s.inc();

   printf("inc done\n");
   sleep(5);

   s.v();
   s.v();

   if (s.check() == success)
      printf("free \n");
   else
      printf("locked \n");

   return 0;
}
