#include <exec/types.h>
#include <intuition/intuition.h>
#include <stdio.h>
#include "machine.h"

/*
 * This is the machine dependent module for graphics on the amiga
 * Functions are -
 *	init_graphics()
 *	exit_graphics()
 *	get_input()
 *	set_pen()
 *	move()
 *	draw()
 *	plot()
 *	clear()
 *	punt()
 *
 * init_graphics() sets global variables x_size, y_size, and max_intensity
 */

/*====================================================================
** Variables
**====================================================================*/

/* Global */
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Screen *screen = NULL;
struct RastPort *rp;

SHORT x_size, y_size, max_intensity, intensity;

/*====================================================================
** Types
**==================================================================*/
struct NewScreen newscreen = {
	0,				/* LeftEdge */
	0,				/* TopEdge */
	MAXPIXELS,		/* Width */
	MAXLINE,		/* Height */
	0,				/* Depth */
	0,				/* DetailPen */
	1,				/* BlockPen */
	NULL,			/* ViewModes */
	CUSTOMSCREEN,	/* Type */
	NULL,			/* Font */
	NULL,			/* DefaultTitle */
	NULL,			/* Gadgets */
	NULL			/* CustomBitMap */
};

/* standard colourmap */
UWORD colormap[16];

/*==================================================================*/
/*********************************************************************
**  init_graphics()
** 
**
*/
void 
init_graphics(
  int req_mode)
{
	long i;
	static UWORD colors[] = {
		0x0000, /* black */
		0x0fff, /* white */
		0x0f00, /* red */
		0x00f0, /* green */
		0x000f, /* blue */
		0x00ff, /* cyan */
		0x0ff0, /* yellow */
		0x0f0f  /* magenta */
	};

	newscreen.Depth = (req_mode == GREYS) ? 4 : 3;
	x_size = MAXPIXELS;
	y_size = MAXLINE;
	max_intensity = (1 << newscreen.Depth) -1;
	intensity = -1;

	/* Amiga Screens */
	if( (IntuitionBase = (struct IntuitionBase *) 
			OpenLibrary("intuition.library", 0L)) == NULL)
		punt("couldn't open intuition");

	if( (GfxBase = (struct GfxBase *) 
			OpenLibrary("graphics.library", 0L)) == NULL)
		punt("couldn't open graphics library");

	if( (screen = (struct Screen *)
			OpenScreen(&newscreen)) == NULL)
		punt("couldn't open screen");

	/* Rastport */
	rp = &(screen->RastPort);
	rp->TmpRas = NULL;		/* small hack for Amiga AreaFill hack */

	/* assign colours (either grey-scale or colours) */
	if(req_mode == GREYS)
	{
		for(i=0; i<=max_intensity; ++i)
			colormap[i] = i | (i<<4) | (i<<8);
	}
	else
	{
		for(i=0; i<8; ++i)
			colormap[i] = colors[i];
	}

	LoadRGB4(&screen->ViewPort, colormap, 16L);

	clear();
}

/*********************************************************************
**  exit_graphics
** 
**  This is called to terminate the graphics env.  If passed a non-null
**  string ot prints that as an error message.  Otherwise you just get
**  the normal exit-from-program message.
*/
void 
exit_graphics(
  char *s)			/* I: error message */
{
	register char c;
	WBenchToFront();
	if(s) printf("%s\n", s);
	printf("Hit RETURN to exit from program (Amiga-M to see picture) -- ");
	while( (c=getchar()) != '\n' && c != EOF);

	if(screen) CloseScreen(screen);
	if(GfxBase) CloseLibrary(GfxBase);
	if(IntuitionBase) CloseLibrary(IntuitionBase);
}

/*********************************************************************
**  get_input
** 
**  returns a line of input in buffer "s".  The prompt "=> " is displayed.
**  If EOF or error is encountered NULL is returned
*/
char* 
get_input(
  char *s)			/* O: message */
{
	printf("=> "); 
	return gets(s);
}

/*********************************************************************
**  punt
** 
**  Takes a string parameter which it passes to exit_graphics and
**  then exits with an error#
*/
void
punt(
  char *s)			/* I: error message */
{
	exit_graphics(s);
	exit(1);
}

/*  Basic interfaces to the Amiga drawing routines */

/*********************************************************************
**  set_pen
** 
**  In this we avoid calling the system routines if the color is 
**  already what is requested.  Note that no clipping is performed
**  so the program may crash if you draw outside the screen.
*/
void
set_pen(
  SHORT new_intensity)			/* I:  */
{
	if(new_intensity != intensity)
	{
		SetAPen(rp, (long) new_intensity);
		intensity = new_intensity;
	}
}

/*********************************************************************
**  move
** 
*/
void
move(
  SHORT x,			/* I:  */
  SHORT y)			/* I:  */
{
	Move(rp, (long) x, (long) y);
}

/*********************************************************************
**  draw
** 
*/
void
draw(
  SHORT x,			/* I:  */
  SHORT y)			/* I:  */
{
	Draw(rp, (long) x, (long) y);
}

/*********************************************************************
**  plot
** 
*/
void
plot(
  SHORT x,			/* I:  */
  SHORT y)			/* I:  */
{
	WritePixel(rp, (long) x, (long) y);
}

/*********************************************************************
**  clear
** 
*/
void clear(void)
{
	SetRast(rp, 0L);
}



/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

void main(void)
{
	int offset;
	init_graphics(COLORS);

	set_pen((SHORT) GREEN);
	for(offset = 10; offset < 20; offset+=2)
	{
		move(offset, offset);
		draw(100,10+offset);
		draw(100,100);
		draw(10+offset,100);
		draw(10+offset,10);
	}

	exit_graphics(NULL);
}
