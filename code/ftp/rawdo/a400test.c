#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include <stdlib.h>
#include <stdio.h>

static void kput1(void);
void __builtin_emit(long int);

int main(void) {
  ULONG array[] = {(ULONG) "Hello world!" };
  TEXT buf[200];
  printf("Starting Printing\n");
  
  RawDoFmt("Print: %s\n", &array, &kput1, buf);
  printf("Printing %s\n", buf);
  return 1;
}

static void kput1(void)
{
  __builtin_emit(0x16C0);	// move.b d0,(a3)+
}
