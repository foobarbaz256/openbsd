#include <stdio.h>

static struct sss{
  int f;
  struct {long m;} snd;
} sss;

#define _offsetof(st,f) ((char *)&((st *) 16)->f - (char *) 16)

int main (void) {
  printf ("+++Struct long inside struct starting with int:\n");
  printf ("size=%d,align=%d\n", sizeof (sss), __alignof__ (sss));
  printf ("offset-int=%d,offset-sss-long=%d,\nalign-int=%d,align-sss-long=%d\n",
          _offsetof (struct sss, f), _offsetof (struct sss, snd),
          __alignof__ (sss.f), __alignof__ (sss.snd));
  return 0;
}
