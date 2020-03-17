/*
    Example for RawDoFmt formatting
*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include <stdlib.h>
#include <stdio.h>

void kput1(void)
{
  __builtin_emit(0x15C0);	// move.b d0,(a3)+
}

int main(void)
{
    /*
        RawDoFmt() expects WORD alignment but the C compiler
        aligns to LONG by default. GCC can be forced to use
        WORD alignment by #pragma pack(2)
    */
    WORD wordval = 1000;
    LONG longval = 100000;
    struct Data
    {
        LONG longval;
        WORD wordval;
        STRPTR str;
    } data = {10000000,1001,"Hello"};

    /* Format string with placeholders. */
    STRPTR formatstring = "TEST LONG %ld WORD %d STRING %s";
    
    /* The data which will be inserted in the placeholders. */
    APTR datastream = &data;
    
    /*  Storage place for result. Note that there is no boundary check. */
    TEXT putchdata[1000]; 
    
    RawDoFmt(
        formatstring, // CONST_STRPTR FormatString
        datastream,   // APTR DataStream
        kput1,         // VOID_FUNC PutChProc
        putchdata     // APTR PutChData
    );

    puts(putchdata);

    
    /*
        The pragma trick doesn't work with variadic functions like
        Printf(), EasyRequest() etc. Here you have to use for integers
        %ld/%lu for the placeholders and the type must be converted
        to LONG/ULONG.
    */
    
    
    Printf("Printf: %ld %ld\n", (LONG)wordval, longval);
    
    return 0;
}
