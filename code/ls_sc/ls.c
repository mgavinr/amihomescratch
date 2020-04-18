/*
 *  LS 4.8lgr
 *
 *  This software is Copyright � 1989, 1990, 1991, 1992  Loren J. Rittle.
 *  This software is Copyright for the sole purpose of protecting
 *  the fact that this is indeed free software.
 *
 *  This software is wholly, but at this point loosely, based upon
 *  Justin V. McCormick's PD LS ``V3.1  July 29, 1989'' with
 *  enhancements and bug fixes by the copyright holder, Loren J. Rittle.
 */

/* This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

See COPYING and ls.doc for copyright and copying information. */

/* If you generate a new version for release to the public,
please replace my initials 'ljr' with your own (so the public
knows who to blame... :-).  Your version should be marked
(at least in the source and documentation) as having been based
upon the 'ljr' strain version <current version number>.

On the other hand, you can send bug reports and patches to
rittle@comm.mot.com for me to incorporate into my next major
release.

Loren J. Rittle
rittle@comm.mot.com */

/* 
   4.8 Sun Apr 12 16:57:08 IST 2020 compiled with sasc 658
   This version based on above, but by insert-name-here
   - updated to print unix/ftp compatible output by default for
         ls -l
     To get the more meaningful amiga output use 
         ls -b
         ls -lb
     see AMIGAFORMAT (in the code that is) I would suggest 
     aliasing ls with that option then in your startup so you can 
     just have one ls binary that works for amiga and unix
   - It also includes fixes for garbage output in ls -l
   - It also appends / for ls without -l
   - It also shows the version number in help
   - Code style changed to google from gnu
   - some Makefile syntax errors fixed
 */

#include "ls.h"

#define MAXARG 100
#define PADTABSIZE 256
#define WORKSIZE 1024

void _CXBRK (int sig);
void _CXBRK (int sig) {}

BYTE version[]="1";
BYTE shortusage[]= "LS Version " VERSION_STRING "\n\
Usage: ls [-h?acdefklnrstw1ADFHIMNOPRTX <args>] [path] ...\n";
BYTE fullusage[]= "\
	a  List all entries	A  Display entries across line\n\
	c  Show filenotes	D  Show dirs last\n\
	d  About dirs		F <format> Format output\n\
	e  Sort by extension	H  No headings and subtotals\n\
	f  Only show files	I <pattern> Ignore pattern\n\
	k  Kill ANSI codes	M  Mixed output files and dirs\n\
	l  Long listing		N <date> Show newer than date\n\
	n  No sort		O <date> Show older than date\n\
	r  Reverse sort		P  Show full pathnames\n\
	s  Sort by size		R  Recursive listing\n\
	t  Sort by date		T  Total all subtotals\n\
	b  Amiga ls -l output\n\
	w <string> Set %%w	X <wide> Set output columns\n\
	1  One entry per line\n";
BYTE deffmtstr[]= "%p %5b %8s %d %n%w%C\\n";
BYTE deffullstr[]= "%n\\n";
BYTE LongFmtStr[]= "%ld";
BYTE totalfmtstr[]= "Dirs:%-4ld Files:%-4ld Blocks:%-5ld Bytes:%-8ld\n";
BYTE smalltotalfmtstr[]= "total %-5ld\n";
BYTE TotHeaderMsg[]= "Totals:\n";
BYTE ErrFmtStr[]= "ls: %s\n";
BYTE BadOptFmtStr[]= "ls: Unknown option -%lc\n";
BYTE OptNeedsArgStr[]= "ls: Option -%lc needs argument\n";
BYTE NoExamFmtStr[]= "ls: Cannot examine file or directory, Error #%ld\n";
BYTE NoFindFmtStr[]= "ls: '%s' not found\n";
BYTE NotGoodDateStr[]= "ls: '%s' not a date\n";
BYTE NoWildPathMsg[]= "Unable to pattern match paths";
BYTE NoRAMMsg[]= "No RAM";
BYTE ParseErrorMsg[]= "ParsePattern Error";
BYTE NameFromLockMsg[]= "NameFromLock Error";
BYTE ExNextMsg[]= "ExNext Error";
BYTE NoCurrentDirMsg[]= "No Current Directory";

/* these are basically related to fib_DirEntryType after being
   remaped by RemapDirEntryType (). */
struct highlight highlight_tabx[13] = {
    /* -7 FILE_DEFAULT */ {"", "", 0},
    /* -6 LINKFILE_EXE */ {"\x9b" "1m", "\x9b" "22m", 0},
    /* -5 FILE_EXE     */ {"", "", 0},
    /* -4 LINKFILE     */ {"\x9b" "1m", "\x9b" "22m", 0},
    /* -3 FILE         */ {"", "", 0},
    /* -2 SOFTLINK_EXE */ {"\x9b" "3m", "\x9b" "23m", 0},
    /* -1 SOFTLINK     */ {"\x9b" "3m", "\x9b" "23m", 0},
    /*  0 COMMENT      */ {"\x9b" "2m/* ", " */\x9b" "22m", 6},
    /*  1 ROOT         */ {"\x9b" "2m", "\x9b" "22m", 0},
    /*  2 USERDIR      */ {"\x9b" "2m", "/\x9b" "22m", 1},
    /*  3 LABEL        */ {"\x9b" "2m", "\x9b" "22m", 0},
    /*  4 LINKDIR      */ {"\x9b" "2;1m", "\x9b" "22m", 0},
    /*  5 DIR_DEFAULT  */ {"", "", 0},
}, *highlight_tab = &highlight_tabx[7];
#define HIGHLIGHT_MAX 5
#define HIGHLIGHT_MIN -7

#define HI_FILE_DEFAULT -7
#define HI_LINKFILE_EXE -6
#define HI_FILE_EXE -5
#define HI_LINKFILE -4
#define HI_FILE -3
#define HI_SOFTLINK_EXE -2
#define HI_SOFTLINK -1
#define HI_COMMENT 0
#define HI_ROOT 1
#define HI_USERDIR 2
#define HI_LABEL 3
#define HI_LINKDIR 4
#define HI_DIR_DEFAULT 5

struct highlight highlight_null = {"", "", 0};
struct highlight highlight_cursor = {"\x9b" " p", "\x9b" "0 p", 0};

struct DateStamp theolddate;
struct DateStamp thenewdate;
struct FileLock *CurFLock;

/* Don't change order of next four entries.  Order needs to be preserved
   for VPrintf calls. */
LONG gdircount;
LONG gfilecount;
LONG gtotblocks;
LONG gtotbytes;
/* end of important order */

/* Don't change order of next four entries.  Order needs to be preserved
   for VPrintf calls. */
LONG dircount;
LONG filecount;
LONG totblocks;
LONG totbytes;
/* end of important order */

LONG madeuptotblocks = 100;
LONG maxnamlen;
enum sortkey {
    sort_alpha, sort_size, sort_date, sort_extension, sort_none
} sortkey;
char *wflag = "\n";
BYTE padtab[PADTABSIZE];
BYTE thePath[WORKSIZE * 2 + 4];
BYTE theDirPat[WORKSIZE];
BYTE theFilePat[WORKSIZE];
BYTE theDirPatParsed[WORKSIZE * 2 + 2];
BYTE theFilePatParsed[WORKSIZE * 2 + 2];
BYTE workstr[WORKSIZE + 64];
BYTE theblksstr[64];
BYTE thedatestr[64];
BYTE theprotstr[64];
BYTE thesizestr[64];
BYTE thehexsizestr[64];
BYTE thetimesizestr[64];
LONG CurWinCols;
BYTE *curpath;
BYTE *thefmtstr = deffmtstr;
struct AnAntiPattern *TheAntiPatterns;

int BREAKFLAG;
int CONSOLE;
int HIDEDIRS;
int LISTALL;
int AMIGAFORMAT;
int LONGLIST;
int NOTEFLAG;
int PATHNAMED;
int REVFLAG;
int ABOUTDIRS;
int FULLPATHNAMES;
int TOTALIZE;
int NOHEADERS;
int FILESFIRST;
int MIXFILESDIRS;
int SHOWOLDERTHAN;
int SHOWNEWERTHAN;
int SHOWHIDDEN;
int ACROSSLIST;

static int CompareExtensions (char *, char *);
static int CmpDateBounds (struct DateStamp *);
static int CompFibs (enum sortkey, struct FileInfoBlock *, struct FileInfoBlock *);
static void WriteErrorString (BYTE *,...);
static void TestBreak (void);
static void DateStr (BYTE *, struct DateStamp *);
static int StrDate (BYTE *, struct DateStamp *);
static void CleanUp (BYTE *, LONG, LONG);
static void AddAntiPattern (BYTE *);
static void Usage (int);
void kput1 (void);
static void GetWinBounds (LONG *, LONG *, LONG);
static int FillFibEntry (struct MinList *, struct FileInfoBlock *);
static struct FibEntry *ModNextFib (struct FibEntry *, LONG);
static void SListDir (struct MinList *);
static void LListEntry (struct FileInfoBlock *);
static void LListDir (struct MinList *);
static void FreeAllFibs (struct MinList *);
static struct MinList *GetDir (struct FileLock *, struct FileInfoBlock *);
static void DirIt (struct FileLock *, BYTE *);
static void GetCLIArgs (BYTE *, LONG *, BYTE **);
static LONG ParseCmdOptions (LONG, LONG, BYTE **);
//static char *stpcpy (char *, char *);
//int stricmp (char *, char *);
static void RemapDirEntryType (struct FileInfoBlock *a);

static void RemapDirEntryType (struct FileInfoBlock *a) {
    /* Remap fib_DirEntryType to allow for easier usage later. */
    /* See order in highlight_tab for more details. */
    switch (a->fib_DirEntryType) {
        case ST_LINKFILE:
        case ST_FILE:
            if ((a->fib_Protection & FIBF_SCRIPT) ||
                    (~a->fib_Protection & FIBF_EXECUTE))
                a->fib_DirEntryType -= 2;
            break;
        case ST_ROOT:
        case ST_USERDIR:
        case ST_LINKDIR:
            break;
        case ST_SOFTLINK:
            if ((a->fib_Protection & FIBF_SCRIPT) ||
                    (~a->fib_Protection & FIBF_EXECUTE))
                a->fib_DirEntryType = HI_SOFTLINK_EXE;
            else
                a->fib_DirEntryType = HI_SOFTLINK;
            break;
        default:
            if (a->fib_DirEntryType >= 0)
                a->fib_DirEntryType = HI_DIR_DEFAULT;
            else
                a->fib_DirEntryType = HI_FILE_DEFAULT;
            break;
    }
}

static int CompareExtensions (char *namea, char *nameb) {
    char *taila = strrchr (namea, '.');
    char *tailb = strrchr (nameb, '.');

    if (taila)
        return tailb ? stricmp (taila, tailb) : 1;
    else
        return tailb ? -1 : 0;
}

static int CmpDateBounds (struct DateStamp *tdate) {
    if ((SHOWNEWERTHAN) && (CompareDates (&thenewdate, tdate) <= 0))
        return 0;
    if ((SHOWOLDERTHAN) && (CompareDates (tdate, &theolddate) <= 0))
        return 0;
    return 1;
}

static int CompFibs (enum sortkey keytype, struct FileInfoBlock *a, struct FileInfoBlock *b) {
    int rc;

    if (((a->fib_DirEntryType < 0) && (b->fib_DirEntryType > 0)) ||
            ((a->fib_DirEntryType > 0) && (b->fib_DirEntryType < 0)))
        if (!(MIXFILESDIRS))
            return (FILESFIRST) ?
                   (a->fib_DirEntryType < 0) :
                   (a->fib_DirEntryType > 0);

    switch (keytype) {
        case sort_size:
            rc = b->fib_Size - a->fib_Size;
            goto like;
        case sort_date:
            rc = CompareDates (&b->fib_Date, &a->fib_Date);
            goto like;
        case sort_extension:
            rc = CompareExtensions (b->fib_FileName, a->fib_FileName);
like:
            if (rc > 0)
                return !REVFLAG;
            if (rc < 0)
                return REVFLAG;
        case sort_alpha:
            if (stricmp (b->fib_FileName, a->fib_FileName) > 0)
                return !REVFLAG;
            else
                return REVFLAG;
        case sort_none:
            return REVFLAG;
        default:
            return REVFLAG;
    }
}

static void WriteErrorString (BYTE *tstring, ...) {
    struct Process *procp = (struct Process *) FindTask (0L);
    BPTR StdErr = procp->pr_CES;

    VFPrintf (StdErr ? StdErr : Output (), tstring, (LONG *) & tstring + 1);
}

static void TestBreak (void) {
    if (CheckSignal (SIGBREAKF_CTRL_C)) {
        PutStr ("*** Break: ls\n");
        BREAKFLAG = 1;
    }
}

static void DateStr (BYTE *s, struct DateStamp *ds) {
    char StrDate[LEN_DATSTRING], StrTime[LEN_DATSTRING];
    struct DateTime dt;
    struct DateStamp cds;

    dt.dat_Stamp = *ds;
    dt.dat_Format = FORMAT_DOS;
    dt.dat_Flags = 0;
    dt.dat_StrDay = NULL;
    dt.dat_StrDate = StrDate;
    dt.dat_StrTime = StrTime;

    if (!DateToStr (&dt)) {
        strcpy (s, "Bad  1  Date");
        return;
    }

    s[0] = StrDate[3];
    s[1] = StrDate[4];
    s[2] = StrDate[5];
    s[3] = ' ';
    s[4] = (StrDate[0] == '0') ? ' ' : StrDate[0];
    s[5] = StrDate[1];
    s[6] = ' ';
    DateStamp (&cds);
    if (((cds.ds_Days - ds->ds_Days) > 180) || ((cds.ds_Days - ds->ds_Days) < 0)) {
        s[7] = ' ';
        if (ds->ds_Days < 8035) {
            s[8] = '1';
            s[9] = '9';
        } else {
            s[8] = '2';
            s[9] = '0';
        }
        s[10] = StrDate[7];
        s[11] = StrDate[8];
    } else {
        s[7] = StrTime[0];
        s[8] = StrTime[1];
        s[9] = ':';
        s[10] = StrTime[3];
        s[11] = StrTime[4];
    }
    s[12] = '\0';
}

static int StrDate (BYTE *s, struct DateStamp *ds) {
    char StrDate[LEN_DATSTRING], StrTime[LEN_DATSTRING];
    struct DateTime dt;

    switch (strlen (s)) {
        case 17:
            /* Standard "MMM DD HH:MM YYYY" 'ls' format */
            if ((StrDate[0] = s[4]) == ' ')
                StrDate[0] = '0';
            StrDate[1] = s[5];
            StrDate[2] = '-';
            StrDate[3] = s[0];
            StrDate[4] = s[1];
            StrDate[5] = s[2];
            StrDate[6] = '-';
            StrDate[7] = s[15];
            StrDate[8] = s[16];
            StrDate[9] = '\0';

            StrTime[0] = s[7];
            StrTime[1] = s[8];
            StrTime[2] = s[9];
            StrTime[3] = s[10];
            StrTime[4] = s[11];
            StrTime[5] = ':';
            StrTime[6] = '0';
            StrTime[7] = '0';
            StrTime[8] = '\0';
            break;
        case 18:
            /* Standard "DD-MMM-YY HH:MM:SS" AmigaDOS 'date' and internal format */
            StrDate[0] = s[0];
            StrDate[1] = s[1];
            StrDate[2] = s[2];
            StrDate[3] = s[3];
            StrDate[4] = s[4];
            StrDate[5] = s[5];
            StrDate[6] = s[6];
            StrDate[7] = s[7];
            StrDate[8] = s[8];
            StrDate[9] = '\0';

            StrTime[0] = s[10];
            StrTime[1] = s[11];
            StrTime[2] = s[12];
            StrTime[3] = s[13];
            StrTime[4] = s[14];
            StrTime[5] = s[15];
            StrTime[6] = s[16];
            StrTime[7] = s[17];
            StrTime[8] = '\0';
            break;
        default:
            return 1;
    }

    dt.dat_Format = FORMAT_DOS;
    dt.dat_Flags = 0;
    dt.dat_StrDay = NULL;
    dt.dat_StrDate = StrDate;
    dt.dat_StrTime = StrTime;

    if (!StrToDate (&dt))
        return 1;

    *ds = dt.dat_Stamp;
    return 0;
}

static void CleanUp (BYTE *exit_msg, LONG exit_status, LONG result2) {
    if (CurFLock)
        UnLock ((BPTR) CurFLock);

    if (*exit_msg)
        WriteErrorString (ErrFmtStr, exit_msg);

    PutStr (highlight_cursor.on);
    Flush (Output ());

    SetIoErr (result2);

    /* this cleanup routine assumes that the C library will free all
       malloc'd memory.  This is true with SAS/C with standard exit(). */
    exit ((int) exit_status);
}

static void AddAntiPattern (BYTE *pattern) {
    struct AnAntiPattern *AnAntiPattern = malloc (sizeof (struct AnAntiPattern));
    int len = strlen (pattern) * 2 + 2;

    if (!AnAntiPattern)
        CleanUp (NoRAMMsg, RETURN_FAIL, ERROR_NO_FREE_STORE);
    AnAntiPattern->next = TheAntiPatterns;
    AnAntiPattern->pattern = pattern;
    if (!(AnAntiPattern->parsedpattern = malloc (len)))
        CleanUp (NoRAMMsg, RETURN_FAIL, ERROR_NO_FREE_STORE);
    if (ParsePatternNoCase (pattern, AnAntiPattern->parsedpattern, len) == -1)
        CleanUp (ParseErrorMsg, RETURN_FAIL, ERROR_NO_MORE_ENTRIES);
    TheAntiPatterns = AnAntiPattern;
}

static void Usage (int full) {
    PutStr (shortusage);
    if (full)
        PutStr (fullusage);
    CleanUp ("", full ? RETURN_WARN : RETURN_FAIL, ERROR_BAD_TEMPLATE);
}

#ifdef __SASC
void kput1 (void) {
    void __builtin_emit (int);

    __builtin_emit (0x16C0);	/* MOVE.B  D0,(A3)+ */
}

#else
#error Fix kput1() for your compiler.
#endif

static void GetWinBounds (LONG *width, LONG *height, LONG have_console) {
    BPTR output = Output ();
    char buffer[16];

    if (have_console && output && IsInteractive (output) &&
            SetMode (output, 1) && (Write (output, "\x9b" "0 q", 4) == 4) &&
            WaitForChar (output, 10000L) &&
            (Read (output, buffer, sizeof (buffer)) > 9) &&
            (buffer[0] == '\x9b')) {
        int y = StrToLong (buffer + 5, height);
        int x = StrToLong (buffer + 5 + y + 1, width);
        if ((x == -1) || (y == -1))
            goto TRY_ENV;
    } else {
TRY_ENV:
        if ((GetVar ("console_width", buffer, sizeof (buffer), LV_VAR | GVF_LOCAL_ONLY) == -1) ||
                (StrToLong (buffer, width) == -1))
            *width = 77;
        if ((GetVar ("console_height", buffer, sizeof (buffer), LV_VAR | GVF_LOCAL_ONLY) == -1) ||
                (StrToLong (buffer, height) == -1))
            *height = 23;
    }
    if (have_console && output && IsInteractive (output))
        SetMode (output, 0);
}

static int FillFibEntry (struct MinList *headfib, struct FileInfoBlock *fibp) {
    struct FibEntry *afib, *tfibp = malloc (sizeof (struct FibEntry));

    if (!tfibp) {
        BREAKFLAG = 1;
        return 0;
    }

    tfibp->fe_Fib = *fibp;

    RemapDirEntryType (&(tfibp->fe_Fib));

    for (afib = (struct FibEntry *) headfib->mlh_Head; afib->fe_Node.mln_Succ;
            afib = (struct FibEntry *) afib->fe_Node.mln_Succ)
        if (CompFibs (sortkey, &(tfibp->fe_Fib), &(afib->fe_Fib)))
            break;
    Insert ((struct List *) headfib, (struct Node *) tfibp, (struct Node *) afib->fe_Node.mln_Pred);

    return 1;
}

static struct FibEntry *ModNextFib (struct FibEntry *tfibp, LONG rows) {
    LONG i;

    for (i = 0; i < rows && tfibp->fe_Node.mln_Succ; i++)
        tfibp = (struct FibEntry *) tfibp->fe_Node.mln_Succ;
    return (tfibp);
}

static void SListDir (struct MinList *fibheadp) {
    LONG avglen;
    LONG colcnt;
    LONG dfcount;
    LONG i, j, wlen;
    LONG maxcol;
    LONG maxrow;
    LONG rowcnt;
    LONG tlen;
    LONG totlen;
    struct FibEntry *hfibp, *tfibp;

    for (totlen = dfcount = 0, hfibp = (struct FibEntry *) fibheadp->mlh_Head;
            hfibp->fe_Node.mln_Succ;
            hfibp = (struct FibEntry *) hfibp->fe_Node.mln_Succ) {
        totlen += strlen (hfibp->fe_Fib.fib_FileName) +
                  highlight_tab[hfibp->fe_Fib.fib_DirEntryType].printable_len;
        dfcount++;
    }

    avglen = totlen / dfcount;
    if (totlen % dfcount)
        avglen++;

    if ((CurWinCols) <= maxnamlen)
        maxcol = 1;
    else
        for (maxcol = 0, colcnt = CurWinCols; colcnt >= avglen; maxcol++)
            colcnt -= avglen + 2;

    for (;;) {
        memset (padtab, 0, PADTABSIZE);

        if (!(ACROSSLIST)) {
            maxrow = dfcount / maxcol;
            if (dfcount % maxcol)
                maxrow++;
            for (rowcnt = 0, hfibp = (struct FibEntry *) fibheadp->mlh_Head;
                    rowcnt < maxrow && hfibp->fe_Node.mln_Succ;
                    rowcnt++, hfibp = (struct FibEntry *) hfibp->fe_Node.mln_Succ) {
                for (colcnt = 0, tfibp = hfibp;
                        colcnt < maxcol && tfibp->fe_Node.mln_Succ;
                        colcnt++, tfibp = ModNextFib (tfibp, maxrow)) {
                    tlen = strlen (tfibp->fe_Fib.fib_FileName) +
                           highlight_tab[tfibp->fe_Fib.fib_DirEntryType].printable_len;
                    if (tlen > padtab[colcnt])
                        padtab[colcnt] = tlen;
                }

                if (!rowcnt) {
                    maxcol = colcnt;
                    maxrow = dfcount / maxcol;
                    if (dfcount % maxcol)
                        maxrow++;
                }
            }
        } else {
            maxrow = dfcount / maxcol;
            if (dfcount % maxcol)
                maxrow++;

            for (rowcnt = 0, tfibp = (struct FibEntry *) fibheadp->mlh_Head;
                    rowcnt < maxrow && tfibp->fe_Node.mln_Succ;
                    rowcnt++) {
                for (colcnt = 0;
                        colcnt < maxcol && tfibp->fe_Node.mln_Succ;
                        colcnt++, tfibp = (struct FibEntry *) tfibp->fe_Node.mln_Succ) {
                    tlen = strlen (tfibp->fe_Fib.fib_FileName) +
                           highlight_tab[tfibp->fe_Fib.fib_DirEntryType].printable_len;
                    if (tlen > padtab[colcnt])
                        padtab[colcnt] = tlen;
                }
            }
        }

        for (colcnt = totlen = 0; (colcnt + 1) < maxcol; colcnt++)
            totlen += (LONG) padtab[colcnt] + 2;
        totlen += (LONG) padtab[colcnt];

        if (maxcol > 1 && totlen > CurWinCols)
            maxcol--;
        else
            break;
    }

    if (!(ACROSSLIST)) {
        for (rowcnt = 0, hfibp = (struct FibEntry *) fibheadp->mlh_Head;
                !(BREAKFLAG) && rowcnt < maxrow && hfibp->fe_Node.mln_Succ;
                TestBreak (), rowcnt++, hfibp = (struct FibEntry *) hfibp->fe_Node.mln_Succ) {
            for (colcnt = 0, tfibp = hfibp;
                    colcnt < maxcol && tfibp->fe_Node.mln_Succ;
                    colcnt++) {
                workstr[0] = 0;
                wlen = strlen (tfibp->fe_Fib.fib_FileName) +
                       highlight_tab[tfibp->fe_Fib.fib_DirEntryType].printable_len;
                strcat (workstr, highlight_tab[tfibp->fe_Fib.fib_DirEntryType].on);
                strcat (workstr, tfibp->fe_Fib.fib_FileName);
                strcat (workstr, highlight_tab[tfibp->fe_Fib.fib_DirEntryType].off);

                tfibp = ModNextFib (tfibp, maxrow);

                if ((colcnt + 1) < maxcol && tfibp->fe_Node.mln_Succ) {
                    for (i = (LONG) padtab[colcnt] + 1, j = strlen (workstr); i >= wlen; i--, j++)
                        workstr[j] = ' ';
                    workstr[j] = 0;
                }
                PutStr (workstr);
            }

            PutStr ("\n");
        }
    } else {
        for (rowcnt = 0, tfibp = (struct FibEntry *) fibheadp->mlh_Head;
                !(BREAKFLAG) && rowcnt < maxrow && tfibp->fe_Node.mln_Succ;
                TestBreak (), rowcnt++) {
            for (colcnt = 0;
                    colcnt < maxcol && tfibp->fe_Node.mln_Succ;
                    colcnt++) {
                workstr[0] = 0;
                wlen = strlen (tfibp->fe_Fib.fib_FileName) +
                       highlight_tab[tfibp->fe_Fib.fib_DirEntryType].printable_len;
                strcat (workstr, highlight_tab[tfibp->fe_Fib.fib_DirEntryType].on);
                strcat (workstr, tfibp->fe_Fib.fib_FileName);
                strcat (workstr, "what");
                strcat (workstr, highlight_tab[tfibp->fe_Fib.fib_DirEntryType].off);

                tfibp = (struct FibEntry *) tfibp->fe_Node.mln_Succ;

                if ((colcnt + 1) < maxcol && tfibp->fe_Node.mln_Succ) {
                    for (i = (LONG) padtab[colcnt] + 1, j = strlen (workstr); i >= wlen; i--, j++)
                        workstr[j] = ' ';
                    workstr[j] = 0;
                }
                PutStr (workstr);
            }

            PutStr ("\n");
        }
    }
}

static void LListEntry (struct FileInfoBlock *fib) {
    BYTE *cp1, *cp2, *reps;
    BYTE *pathend, *thenamestr;
    LONG i, spccnt;

    // valid lunix: -rwxrwxrwx
    // valid amiga: -----rwed
    if (AMIGAFORMAT) {
        theprotstr[0] = "sL-L-ll-dd-DS"[fib->fib_DirEntryType + 7];
        theprotstr[1] = (fib->fib_Protection & 128 /*FIBF_HIDDEN*/ )? 'h' : '-';
        theprotstr[2] = (fib->fib_Protection & FIBF_SCRIPT) ? 's' : '-';
        theprotstr[3] = (fib->fib_Protection & FIBF_PURE) ? 'p' : '-';
        theprotstr[4] = (fib->fib_Protection & FIBF_ARCHIVE) ? 'a' : '-';
        theprotstr[5] = (fib->fib_Protection & FIBF_READ) ? '-' : 'r';
        theprotstr[6] = (fib->fib_Protection & FIBF_WRITE) ? '-' : 'w';
        theprotstr[7] = (fib->fib_Protection & FIBF_EXECUTE) ? '-' : 'e';
        theprotstr[8] = (fib->fib_Protection & FIBF_DELETE) ? '-' : 'd';
        theprotstr[9] = ' ';
        theprotstr[10] = fib->fib_Comment[0] ? 'c' : ' ';
    } else {
        theprotstr[0] = "sL-L-ll-dd-DS"[fib->fib_DirEntryType + 7];
        theprotstr[1] = 'r';
        theprotstr[2] = 'w';
        theprotstr[3] = 'x';
        theprotstr[4] = 'r';
        theprotstr[5] = 'w';
        theprotstr[6] = 'x';
        theprotstr[7] = 'r';
        theprotstr[8] = 'w';
        theprotstr[9] = 'x';
        theprotstr[10] = ' ';
    }

    DateStr (thedatestr, &(fib->fib_Date));

    // TODO this produces garbage for me
    //RawDoFmt (LongFmtStr, &fib->fib_NumBlocks, kput1, theblksstr);
    //RawDoFmt (LongFmtStr, &fib->fib_Size, kput1, thesizestr);
    sprintf(thesizestr, "%ld", fib->fib_Size);
    if (AMIGAFORMAT) {
        sprintf(theblksstr, "%ld", fib->fib_NumBlocks);
    } else {
        sprintf(theblksstr, "%ld %ld", 1, -2);
    }

    i = strlen (curpath);
    pathend = curpath + i;
    if (i > 1 && *(curpath + i - 1) != ':') {
        *(curpath + i) = '/';
        i++;
        *(curpath + i) = 0;
    }
    thenamestr = curpath + strlen (curpath);
    cp2 = thenamestr;
    if ((fib->fib_DirEntryType == HI_SOFTLINK) ||
            (fib->fib_DirEntryType == HI_SOFTLINK_EXE)) {
        struct DevProc *devproc = GetDeviceProc (curpath, NULL);
        UBYTE buffer[512];
        BOOL success;

        cp2 = stpcpy (cp2, fib->fib_FileName);

        if (devproc) {
            cp2 = stpcpy (cp2, " -> ");
            success = ReadLink (devproc->dvp_Port, devproc->dvp_Lock,
                                fib->fib_FileName, buffer, 512);
            if (success)
                strcpy (cp2, buffer);
            else
                strcpy (cp2, "!ERROR READING LINK!");
            FreeDeviceProc (devproc);
        } else
            strcpy (cp2, "!ERROR LOCKING LINK!");
    } else if ((fib->fib_DirEntryType == HI_LINKDIR) ||
               (fib->fib_DirEntryType == HI_LINKFILE) ||
               (fib->fib_DirEntryType == HI_LINKFILE_EXE)) {
        UBYTE buffer[512];
        BPTR plock, lock;

        plock = Lock (curpath, ACCESS_READ);
        plock = CurrentDir (plock);
        cp2 = stpcpy (cp2, fib->fib_FileName);
        cp2 = stpcpy (cp2, " -> ");
        lock = Lock (fib->fib_FileName, ACCESS_READ);
        if (lock) {
            NameFromLock (lock, buffer, 512);
            UnLock (lock);
            strcpy (cp2, buffer);
        } else
            strcpy (cp2, "!ERROR LOCKING LINK!");
        plock = CurrentDir (plock);
        UnLock (plock);
    } else
        strcpy (cp2, fib->fib_FileName);

    for (cp1 = workstr, cp2 = thefmtstr; *cp2; cp2++) {
        if (*cp2 != '%' && *cp2 != '\\')
            *cp1++ = *cp2;
        else {
            if (*cp2++ == '%') {
                if ((*cp2 >= '0') && (*cp2 <= '9')) {
                    cp2 += StrToLong (cp2, &spccnt);
                    if (spccnt > 99)
                        spccnt = 99;
                } else
                    spccnt = 0;

                switch (*cp2) {
                    case 'p':
                        reps = theprotstr;
                        break;
                    case 'd':
                        reps = thedatestr;
                        break;
                    case 'b':
                        reps = theblksstr;
                        break;
                    case 's':
                        reps = thesizestr;
                        break;
                    case 'C':
                        if ((NOTEFLAG) && fib->fib_Comment[0]) {
                            cp1 = stpcpy (cp1, highlight_tab[HI_COMMENT].on);
                            cp1 = stpcpy (cp1, fib->fib_Comment);
                            cp1 = stpcpy (cp1, highlight_tab[HI_COMMENT].off);
                        }
                        reps = "";
                        break;
                    case 'w':
                        if ((NOTEFLAG) && fib->fib_Comment[0])
                            reps = wflag;
                        else
                            reps = "";
                        break;
                    case 'h':
                        RawDoFmt ("0x%lx", &fib->fib_Size, kput1, thehexsizestr);
                        reps = thehexsizestr;
                        break;
                    case 'S': {
                        long xfer_rate;
                        int time[2];

                        cp2++;
                        if ((*cp2 >= '0') && (*cp2 <= '9'))
                            cp2 += StrToLong (cp2, &xfer_rate);
                        else
                            xfer_rate = 0;
                        cp2--;
                        if (!xfer_rate)
                            xfer_rate = 230;

                        time[0] = fib->fib_Size / xfer_rate / 60;
                        time[1] = fib->fib_Size / xfer_rate % 60;
                        RawDoFmt ("%ld:%02ld", time, kput1, thetimesizestr);
                        reps = thetimesizestr;
                        break;
                    }
                    case 'n':
                        if (FULLPATHNAMES)
                            reps = curpath;
                        else
                            reps = thenamestr;
                        break;
                    case '%':
                        *cp1++ = '%';
                        *cp1++ = 0;
                    default:
                        reps = "";
                        break;
                }
                for (i = strlen (reps); i < spccnt; i++)
                    *cp1++ = ' ';
                cp1 = stpcpy (cp1, reps);
            } else {
                switch (*cp2) {
                    case 'n':
                        *cp1++ = '\n';
                        break;
                    case '"':
                        *cp1++ = '"';
                        break;
                    case 't':
                        *cp1++ = '\t';
                        break;
                    case '\\':
                        *cp1++ = '\\';
                        break;
                    default:
                        break;
                }
            }
        }
    }
    *cp1 = 0;
    PutStr (workstr);

    *pathend = 0;
}

static void LListDir (struct MinList *fibheadp) {
    struct FibEntry *tfibp;

    totblocks = totbytes = 0;
    for (tfibp = (struct FibEntry *) fibheadp->mlh_Head;
            tfibp->fe_Node.mln_Succ;
            tfibp = (struct FibEntry *) tfibp->fe_Node.mln_Succ) {
        TestBreak ();
        if (BREAKFLAG)
            return;
        LListEntry (&(tfibp->fe_Fib));
        if (tfibp->fe_Fib.fib_DirEntryType < 0) {
            totblocks += tfibp->fe_Fib.fib_NumBlocks;
            totbytes += tfibp->fe_Fib.fib_Size;
        }
    }

    if (AMIGAFORMAT) {
        if (!(BREAKFLAG || NOHEADERS)) {
            VPrintf (totalfmtstr, &dircount);	/* note that parameters are in order in memory */
        }
    }
}

static void FreeAllFibs (struct MinList *fibheadp) {
    struct FibEntry *tfibp;

    if (fibheadp) {
        while (fibheadp->mlh_Head->mln_Succ) {
            tfibp = (struct FibEntry *) RemTail ((struct List *) fibheadp);
            free (tfibp);
        }
    }
}

static struct MinList *GetDir (struct FileLock *lockp, struct FileInfoBlock *fibp) {
    struct AnAntiPattern *theanti;
    LONG matchstat;
    LONG nextstat;
    LONG tempnamlen;
    struct MinList *fibhead;
    struct MinList *dirhead;

    maxnamlen = dircount = filecount = 0L;

    if (!(fibhead = malloc (sizeof (struct MinList))))
        return (0L);
    NewList ((struct List *) fibhead);

    if (!(dirhead = malloc (sizeof (struct MinList))))
        goto BADALLOC;
    NewList ((struct List *) dirhead);

    do {
        TestBreak ();
        if (BREAKFLAG)
            goto GOODRET;

        if (nextstat = ExNext ((BPTR) lockp, fibp)) {
            if (CmpDateBounds (&fibp->fib_Date)) {
                if ((fibp->fib_DirEntryType >= 0) &&
                        (!(fibp->fib_DirEntryType == ST_SOFTLINK)))
                    matchstat = MatchPatternNoCase (theDirPatParsed, fibp->fib_FileName);
                else
                    matchstat = MatchPatternNoCase (theFilePatParsed, fibp->fib_FileName);

                if (!(SHOWHIDDEN) && matchstat)
                    matchstat = !(fibp->fib_Protection & 128 /*FIBF_HIDDEN*/ );
                theanti = TheAntiPatterns;
                while (matchstat && theanti) {
                    matchstat = !MatchPatternNoCase (theanti->parsedpattern, fibp->fib_FileName);
                    theanti = theanti->next;
                }

                if (!matchstat)
                    continue;

                if ((fibp->fib_DirEntryType >= 0) &&
                        (!(fibp->fib_DirEntryType == ST_SOFTLINK))) {
                    if (!FillFibEntry (dirhead, fibp))
                        goto BADALLOC;
                    if (HIDEDIRS)
                        continue;
                    dircount++;
                    gdircount++;
                } else {
                    filecount++;
                    gfilecount++;
                    gtotblocks += fibp->fib_NumBlocks;
                    gtotbytes += fibp->fib_Size;
                }

                tempnamlen = strlen (fibp->fib_FileName);
                if (tempnamlen > maxnamlen)
                    maxnamlen = tempnamlen;

                if (!FillFibEntry (fibhead, fibp))
                    goto BADALLOC;
            }
        }
    } while (nextstat);

    if (IoErr ()!= ERROR_NO_MORE_ENTRIES)
        CleanUp (ExNextMsg, RETURN_FAIL, IoErr ());

    if (dircount + filecount) {
        if (!(LONGLIST))
            SListDir (fibhead);
        else
            LListDir (fibhead);
    }

GOODRET:
    FreeAllFibs (fibhead);
    return (dirhead);

BADALLOC:
    FreeAllFibs (fibhead);
    FreeAllFibs (dirhead);
    return (0L);
}

static void DirIt (struct FileLock *curlock, BYTE *dirname) {
    BYTE *subdir;
    size_t dnamlen;
    struct FibEntry *tfibp;
    struct FileLock *sublock;
    struct MinList *fibheadp;
    struct FileInfoBlock __aligned GFibp;

    if (!Examine ((BPTR) curlock, &GFibp)) {
        WriteErrorString (NoExamFmtStr, IoErr ());
        return;
    }

    if (dirname[0] && (LISTALL) && !(NOHEADERS)) {
        PutStr (highlight_tab[HI_LABEL].on);
        PutStr (dirname);
        PutStr ("\n");
        PutStr (highlight_tab[HI_LABEL].off);
    }

    if ((GFibp.fib_DirEntryType < 0) ||
            (GFibp.fib_DirEntryType == ST_SOFTLINK) || (ABOUTDIRS)) {
        RemapDirEntryType (&GFibp);

        *(PathPart (thePath)) = '\0';
        LListEntry (&GFibp);
        filecount++;
        gfilecount++;
        gtotblocks += GFibp.fib_NumBlocks;
        gtotbytes += GFibp.fib_Size;
    } else {
        if (fibheadp = GetDir (curlock, &GFibp)) {
            if (LISTALL) {
                for (tfibp = (struct FibEntry *) fibheadp->mlh_Head;
                        tfibp->fe_Node.mln_Succ;
                        tfibp = (struct FibEntry *) tfibp->fe_Node.mln_Succ) {
                    TestBreak ();
                    if (BREAKFLAG)
                        break;

                    dnamlen = (strlen (dirname) + strlen (tfibp->fe_Fib.fib_FileName) + 36);
                    if (subdir = malloc (dnamlen)) {
                        if (dirname[0]) {
                            strcpy (subdir, dirname);
                            dnamlen = strlen (dirname) - 1;
                            if (dirname[dnamlen] != ':' && dirname[dnamlen] != '/')
                                strcat (subdir, "/");
                        }
                        strcat (subdir, tfibp->fe_Fib.fib_FileName);

                        if (sublock = (struct FileLock *) Lock (subdir, (LONG) ACCESS_READ)) {
                            if (!(NOHEADERS))
                                PutStr ("\n");

                            curpath = subdir;

                            DirIt (sublock, subdir);

                            UnLock ((BPTR) sublock);
                        }
                        free (subdir);
                    }
                }
            }
            FreeAllFibs (fibheadp);
        }
    }
}

static void GetCLIArgs (BYTE *line, LONG *argc, BYTE **argv) {
    BYTE **pargv, *qarg;

    *argc = 0;
    while (*argc < MAXARG) {
        while (*line == ' ' || *line == '\t' || *line == '\n')
            line++;
        if (!(*line))
            break;
        pargv = &argv[*argc];
        *argc += 1;

        if (*line == '"') {
            qarg = line;
            line += 1;
            *pargv = line;
            while (*line && *line != '"')
                if (*line == '\\' && *(line + 1))
                    line += 2;
                else
                    line++;
            if (!(*line)) {
                *pargv = qarg;
                break;
            } else
                *line++ = 0;
        } else {
            *pargv = line;
            while (*line && !(*line == ' ' || *line == '\t' || *line == '\n'))
                line++;
            if (*line)
                *line++ = 0;
            else
                break;
        }
    }
}

static LONG ParseCmdOptions (LONG ncnt, LONG argc, BYTE **argv) {
    LONG i, cnt, len;

    cnt = ncnt;
    ncnt += 1;

    for (i = 1, len = strlen (argv[cnt]); i < len; i++) {
        switch (argv[cnt][i]) {
            case '?':
            case 'h':
                Usage (1);
                break;
            case 'D':
                FILESFIRST = 1;
                break;
            case 'F':
                if (argc < (ncnt + 1))
                    goto missing_arg_error;
                thefmtstr = argv[ncnt];
                ncnt += 1;
                LONGLIST = 1;
                break;
            case 'H':
                NOHEADERS = 1;
                break;
            case 'A':
                ACROSSLIST = 1;
                break;
            case 'M':
                MIXFILESDIRS = 1;
                break;
            case 'N':
                if (argc < (ncnt + 1))
                    goto missing_arg_error;
                if (StrDate (argv[ncnt], &thenewdate))
                    goto date_arg_error;
                ncnt += 1;
                SHOWNEWERTHAN = 1;
                break;
            case 'O':
                if (argc < (ncnt + 1))
                    goto missing_arg_error;
                if (StrDate (argv[ncnt], &theolddate))
                    goto date_arg_error;
                ncnt += 1;
                SHOWOLDERTHAN = 1;
                break;
date_arg_error:
                WriteErrorString (NotGoodDateStr, argv[ncnt]);
                CleanUp ("", RETURN_FAIL, ERROR_BAD_TEMPLATE);
            case 'P':
                if ((thefmtstr == deffmtstr) && !(LONGLIST))
                    thefmtstr = deffullstr;
                FULLPATHNAMES = 1;
                LONGLIST = 1;
                NOHEADERS = 1;
                break;
            case 'R':
                LISTALL = 1;
                break;
            case 'T':
                TOTALIZE = 1;
                break;
            case 'X':
                if (argc < (ncnt + 1))
                    goto missing_arg_error;
                StrToLong (argv[ncnt], &CurWinCols);
                ncnt += 1;
                break;
            case 'w':
                if (argc < (ncnt + 1))
                    goto missing_arg_error;
                wflag = argv[ncnt];
                ncnt += 1;
                break;
            case '1':
                CurWinCols = 1;
                break;
            case 'Y':
                if (argc < (ncnt + 1))
                    goto missing_arg_error;
                ncnt += 1;
                break;
            case 'a':
                SHOWHIDDEN = 1;
                break;
            case 'b':
                AMIGAFORMAT = 1;
                break;
            case 'c':
                LONGLIST = 1;
                NOTEFLAG = 1;
                break;
            case 'd':
                ABOUTDIRS = 1;
                break;
            case 'e':
                sortkey = sort_extension;
                break;
            case 'f':
                HIDEDIRS = 1;
                break;
            case 'k':
                CONSOLE = 0;
                break;
            case 'l':
                if ((thefmtstr == deffullstr) && (FULLPATHNAMES))
                    thefmtstr = deffmtstr;
                LONGLIST = 1;
                break;
            case 'n':
                sortkey = sort_none;
                break;
            case 'r':
                REVFLAG = 1;
                break;
            case 's':
                sortkey = sort_size;
                break;
            case 't':
                sortkey = sort_date;
                break;
            case 'I':
                if (argc < (ncnt + 1))
                    goto missing_arg_error;
                AddAntiPattern (argv[ncnt]);
                ncnt += 1;
                break;
            default:
                WriteErrorString (BadOptFmtStr, argv[cnt][i]);
                Usage (0);
missing_arg_error:
                WriteErrorString (OptNeedsArgStr, argv[cnt][i]);
                Usage (1);
        }
    }
    return (ncnt);
}

void __stdargs __main (char *line) {
    BYTE *argv[MAXARG];
    LONG argc;
    LONG cnt = 1;
    LONG Columns, Rows;
    SHORT namedPath = FALSE;

    if (DOSBase->dl_lib.lib_Version < 37) {
        Write (Output (), "ls: Need AmigaOS Release 2.04\n", 30);
        exit (20);
    }

    GetCLIArgs (line, &argc, argv);

    if (IsInteractive (Output ()))
        CONSOLE = 1;

    while (cnt < argc && argv[cnt][0] == '-') {
        if (argv[cnt][1] == '-' || argv[cnt][1] == '\0') {
            cnt++;
            break;
        } else {
            cnt = ParseCmdOptions (cnt, argc, argv);
            continue;
        }
    }

    {
        int i;
        char a[32], *b;

        if (!(b = malloc (32)))
            CleanUp (NoRAMMsg, RETURN_FAIL, ERROR_NO_FREE_STORE);
        for (i = HIGHLIGHT_MIN; i <= HIGHLIGHT_MAX; i++) {
            RawDoFmt ("ls_highlight%ld.on", &i, kput1, a);
            if (GetVar (a, b, 32, LV_VAR | GVF_LOCAL_ONLY) != -1) {
                highlight_tab[i].on = b;
                highlight_tab[i].printable_len = 0;
                for (; *b; b++) {
                    if ((*b == '\x9b') || (*b == '\x1b')) {
                        while (*++b)
                            if (*b == 'm')
                                break;
                    } else
                        highlight_tab[i].printable_len++;
                }
                if (!(b = malloc (32)))
                    CleanUp (NoRAMMsg, RETURN_FAIL, ERROR_NO_FREE_STORE);
                RawDoFmt ("ls_highlight%ld.off", &i, kput1, a);
                if (GetVar (a, b, 32, LV_VAR | GVF_LOCAL_ONLY) != -1) {
                    highlight_tab[i].off = b;
                    for (; *b; b++) {
                        if ((*b == '\x9b') || (*b == '\x1b')) {
                            while (*++b)
                                if (*b == 'm')
                                    break;
                        } else
                            highlight_tab[i].printable_len++;
                    }
                    if (!(b = malloc (32)))
                        CleanUp (NoRAMMsg, RETURN_FAIL, ERROR_NO_FREE_STORE);
                }
            }
        }
        free (b);
    }

    if (!(CONSOLE)) {
        int i;

        for (i = HIGHLIGHT_MIN; i <= HIGHLIGHT_MAX; i++)
            if ((*highlight_tab[i].on == '\x9b') ||
                    (*highlight_tab[i].on == '\x1b') ||
                    (*highlight_tab[i].off == '\x9b') ||
                    (*highlight_tab[i].off == '\x1b'))
                highlight_tab[i] = highlight_null;

        highlight_cursor = highlight_null;
        highlight_tab[HI_COMMENT].on = "/* ";
        highlight_tab[HI_COMMENT].off = " */";
    }

    {
        char b[32];

        if (!(SHOWHIDDEN))
            if (GetVar ("ls_hidepattern", b, sizeof b, LV_VAR | GVF_LOCAL_ONLY) != -1)
                AddAntiPattern (b);
            else
                AddAntiPattern ("(#?.(info|bak)|.#?)");
    }

    if (!CurWinCols) {
        GetWinBounds (&Columns, &Rows, (CONSOLE));
        CurWinCols = Columns;
    }

    PutStr (highlight_cursor.off);

    if (AMIGAFORMAT) {
        ;
    } else {
        if (LONGLIST) {
            VPrintf (smalltotalfmtstr, &madeuptotblocks);	/* note that parameters are in order in memory */
        }
    }
    while (!(BREAKFLAG)) {
        if (cnt < argc) {
            namedPath = TRUE;

            theFilePat[0] = 0;
            theDirPat[0] = 0;
            strcpy (thePath, argv[cnt]);
            cnt++;

            if (ParsePatternNoCase (thePath, theDirPatParsed, sizeof (theDirPatParsed))) {
                strcpy (theFilePat, FilePart (thePath));
                strcpy (theDirPat, theFilePat);
                *(PathPart (thePath)) = '\0';

                if (ParsePatternNoCase (thePath, theDirPatParsed, sizeof (theDirPatParsed))) {
                    strcpy (theDirPat, FilePart (thePath));
                    *(PathPart (thePath)) = '\0';
                }
            }

            if (ParsePatternNoCase (thePath, theDirPatParsed, sizeof (theDirPatParsed)))
                CleanUp (NoWildPathMsg, RETURN_WARN, ERROR_NO_MORE_ENTRIES);

            if (!(CurFLock = (struct FileLock *) Lock (thePath, (LONG) ACCESS_READ))) {
                WriteErrorString (NoFindFmtStr, thePath);
                CleanUp ("", RETURN_FAIL, IoErr ());
            }
        } else if ((!namedPath) && (cnt == argc)) {
            struct Process *procp = (struct Process *) FindTask (0L);
            // it goes here

            cnt++;

            if (procp->pr_CurrentDir)
                CurFLock = (struct FileLock *) DupLock (procp->pr_CurrentDir);
            else
                CurFLock = (struct FileLock *) Lock ("sys:", ACCESS_READ);
            if (!CurFLock)
                CleanUp (NoCurrentDirMsg, RETURN_FAIL, ERROR_NO_DEFAULT_DIR);
        } else
            break;

        if (!strchr (thePath, ':'))
            if (!NameFromLock ((BPTR) CurFLock, thePath, sizeof (thePath)))
                CleanUp (NameFromLockMsg, RETURN_FAIL, IoErr ());
        curpath = thePath;

        if (!theDirPat[0])
            strcpy (theDirPat, "#?");

        if (!theFilePat[0])
            strcpy (theFilePat, "#?");

        if (ParsePatternNoCase (theDirPat, theDirPatParsed, sizeof (theDirPatParsed)) == -1)
            CleanUp (ParseErrorMsg, RETURN_FAIL, ERROR_NO_MORE_ENTRIES);
        if (ParsePatternNoCase (theFilePat, theFilePatParsed, sizeof (theFilePatParsed)) == -1)
            CleanUp (ParseErrorMsg, RETURN_FAIL, ERROR_NO_MORE_ENTRIES);

        DirIt (CurFLock, thePath);

        if (CurFLock)
            UnLock ((BPTR) CurFLock);
        CurFLock = NULL;

        TestBreak ();
    }

    if ((TOTALIZE)) {
        PutStr (highlight_tab[HI_LABEL].on);
        PutStr (TotHeaderMsg);
        PutStr (highlight_tab[HI_LABEL].off);
        VPrintf (totalfmtstr, &gdircount);	/* note that parameters are in order in memory */
        if (AMIGAFORMAT) {
            VPrintf (smalltotalfmtstr, &gtotblocks);	/* note that parameters are in order in memory */
        }
    }

    CleanUp ("", RETURN_OK, 0L);
}
