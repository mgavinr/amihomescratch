/* 	
 * Defines for machine-dependent information
 */

/* compiler */
#define LATTICE 1
#define AZTEC   0

/* short size = 16bits */
#define SHORT short

/* bug */
#define ZERO

/* float is double for lattice */
#if LATTICE
#define FLOAT double
#else
#define FLOAT float
#endif

/* init_graphics() stuff */
#define GREYS 0
#define COLORS 1

#define BLACK   0
#define WHITE   1
#define RED     2
#define GREEN   3
#define BLUE    4
#define CYAN    5
#define YELLOW  6
#define MAGENTA 7

extern SHORT x_size, y_size, max_intensity;
void init_graphics(int);
void exit_graphics(char*);
char* get_input(char*);
void punt(char*);
void set_pen(SHORT);
void move(SHORT, SHORT);
void draw(SHORT, SHORT);
void plot(SHORT, SHORT);
void clear(void);


#define MAXLINE   200
#define MAXPIXELS 320
