#include <stdio.h>

static struct sss{
  short f;
  struct {int m;} snd;
} sss;

#define _offsetof(st,f) ((char *)&((st *) 16)->f - (char *) 16)

int main (void) {
  printf ("+++Struct int inside struct starting with short:\n");
  printf ("size=%d,align=%d\n", sizeof (sss), __alignof__ (sss));
  printf ("offset-short=%d,offset-sss-int=%d,\nalign-short=%d,align-sss-int=%d\n",
          _offsetof (struct sss, f), _offsetof (struct sss, snd),
          __alignof__ (sss.f), __alignof__ (sss.snd));
  return 0;
}
