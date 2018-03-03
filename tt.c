

// g++ -ggdb tt.c config.c lib/config.c lib/common.c -pthread -o tt

#include "config.h"

const char* logPrefix = "";

int main()
{
   cOsd2WebConfig one;
   cOsd2WebConfig two(one);

   printf("test .....\n");

   one.setConfPath("test");
   two.setConfPath("foobar");

   printf("[%s / %s] (%p / %p)\n", two.confPath, one.confPath, two.confPath, &one.confPath);

   one = two;

   printf("[%s / %s] (%p / %p)\n", two.confPath, one.confPath, two.confPath, &one.confPath);

   return 0;
}
