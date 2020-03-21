# Makefile for LS 4.7ljr under the AmigaOS Release 2.04
#
# Set up for SAS/C v6.0 with smake
#
# Last update: Loren J. Rittle  Sun Aug 23 15:18:59 1992

VERSION = 4.7

# Standard Amiga System Shell
#RESIDENT = resident
#DELETE = delete force quiet
#COPY = copy clone
#ECHO = echo

# The WShell 2.0
RESIDENT = resi
DELETE = delete force quiet
COPY = copy clone
ECHO = echo

# Standard version
CFLAGS = ignore=306 utillib
LFLAGS = $(CFLAGS) link batch smallcode smalldata startup=cres

# Standard '030 version
#CFLAGS = ignore=306 cpu=30 #debug=full noopt noautoreg
#LFLAGS = $(CFLAGS) link batch smallcode smalldata startup=cres #debug=full

# Standard '040 version
#CFLAGS = ignore=306 cpu=40 #debug=full noopt noautoreg
#LFLAGS = $(CFLAGS) link batch smallcode smalldata startup=cres #debug=full

OBJS = ls.o
GST = gst=ls.gst
DEST = lcs

$(DEST): $(OBJS)
	$(CC) $(LFLAGS) obj $(OBJS) pname $(DEST)
	$(DELETE) lcs.lnk
	@$(ECHO) " "
	@$(ECHO) "Made $(DEST) be sure that you don't have $(DEST)"
	@$(ECHO) "resident in order to properly test it!"
	@$(ECHO) "Type make install to copy $(DEST) to c:ls"
	@$(ECHO) "and re-resident it."

ls.gst: ls.h
	$(CC) $(CFLAGS) make$(GST) <<(ls_gst.c)
\#include "ls.h"
<
	@$(DELETE) ls_gst.o

ls.o: ls.h ls.gst ls.c
	$(CC) $(CFLAGS) DEFINE VERSION_STRING="$(VERSION)" \
	DEFINE "DATE_STRING=*"`rx REXX/date`*"" \
	DEFINE SHORT_DATE_STRING="`rexx/sdate`" $(GST) ls.c

install: $(DEST)
	@$(ECHO) "Installing $(DEST) as c:lsc"
	@$(COPY) $(DEST) c:lsc
	$(RESIDENT) c:lsc

clean:
	@$(ECHO) "Cleaning up"
	@$(DELETE) \#?.o \#?.bak ls.gst $(DEST)
