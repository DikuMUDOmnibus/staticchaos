#include <stdio.h>
#include <time.h>

int main()
{
  time_t t;
  time( &t );
  printf( "%d\n", (int)t );
  printf( "%s", ctime( &t ) );
  return 0;
}

