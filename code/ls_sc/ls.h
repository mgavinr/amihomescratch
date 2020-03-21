/* Last update: Loren J. Rittle  Sun Aug 23 15:26:36 1992 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma msg 148 ignore push
#pragma msg 149 ignore push
#pragma msg 61 ignore push
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/var.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#pragma msg 149 pop
#pragma msg 61 pop

/* Structure used to hold file info in a linked list */
struct FibEntry
{
  struct MinNode fe_Node;
  struct FileInfoBlock fe_Fib;
};

/* Structure used to hold AntiPattern info in a linked list */
struct AnAntiPattern
{
  struct AnAntiPattern *next;
  BYTE *parsedpattern;
  BYTE *pattern;
};

/* Structure used to hold type and protection highlight info in an array */
struct highlight
{
  unsigned char *on;
  unsigned char *off;
  int printable_len;
};
