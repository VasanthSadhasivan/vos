#ifndef SNAKEH
#define SNAKEH

#define NULL 0

typedef enum {NW,N,NE,W,E,SW,S,SE} direction;
#define NUMDIRS 8

typedef struct point_t {
  int x;
  int y;
} sn_point;

typedef struct snake_st {
  direction       dir;
  int             len;
  int             color;        /* color pair to use if colored snakes */
  sn_point        *body;
  int             pid;       /* useful for playing with scheduling */
  struct snake_st *others;      /* a utility link to find all snakes again */
} *snake;

/* Colors range from 1 (blue on black) to 8 ( black on black).
 */
#define MAX_VISIBLE_SNAKE 7

extern int          start_windowing();
extern void         end_windowing();
extern snake        new_snake(int y, int x, int len, int dir, int color) ;
extern void         free_snake(snake s);
extern void         draw_all_snakes();
extern void         run_snake(void *s);
extern void         run_hungry_snake(void *s);
extern void         kill_snake();
extern unsigned int get_snake_delay();
extern void         set_snake_delay(unsigned int msec);
extern snake        snakeFromLWpid(int lw_pid);
extern void setup_snakes(int);

#define VGA_BLACK 0x00
#define VGA_BLUE 0x01
#define VGA_GREEN 0x02
#define VGA_CYAN 0x03
#define VGA_RED 0x04
#define VGA_PURPLE 0x05
#define VGA_ORANGE 0x06
#define VGA_LIGHT_GREY 0x07
#define VGA_DARK_GREY 0x08
#define VGA_BRIGHT_BLUE 0x09
#define VGA_BRIGHT_GREEN 0x0A
#define VGA_BRIGHT_CYAN 0x0B
#define VGA_MAGENTA 0x0C
#define VGA_BRIGHT_PURPLE 0x0D
#define VGA_YELLOW 0x0E
#define VGA_WHITE 0x0F

#endif
