#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include <stdlib.h>
#include <stdio.h>
/*
 WORD = 16 bits (this is int, default for sc)
 1111 1111 1111 1111 = 16 bits
    E    E    E    E
 LONG = 32 bits
 1111 1111 1111 1111  1111 1111 1111 1111
    E    E    E    E     E    E    E    E
*/

struct t_data {
  int thevalue;
};
typedef struct t_data _t_data;
void kput1(void);
void __builtin_emit(int);

int main(void) {
  _t_data in_data;
  TEXT out_data[1000];
  int i=0;

  memset(in_data, 0, sizeof(_t_data));
  memset(out_data, 0, sizeof(out_data));

  in_data.thevalue = 99;
  RawDoFmt("1234%d\n", &in_data, &kput1, out_data);
  printf("RESULT>>%s<<\n", out_data);
  return 1;
}

void kput1(void)
{
  __builtin_emit(0x16C0);	// move.b d0,(a3)+
}
