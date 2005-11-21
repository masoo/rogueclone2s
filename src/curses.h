#ifndef __CURSES_H__
#define __CURSES_H__

extern void initscr(void);
extern void endwin(void);
extern void repaint_screen(void);
extern void move(short row, short col);
extern void mvaddstr(short row, short col, char *str);
extern void addstr(char *str);
extern void addch(int ch);
extern void mvaddch(short row, short col, int ch);
extern void touch(int row, int bcol, int ecol);
extern void refresh(void);
extern void wrefresh(WINDOW *scr);
extern int mvinch(short row, short col);
extern void clear(void);
extern void clrtoeol(void);
extern void color(short col);
extern void standout(void);
extern void standend(void);
extern void crmode(void);
extern void noecho(void);
extern void nonl(void);
extern void clear_buffers(void);
extern void put_cursor(int row, int col);
extern void put_st_char(int ch);
extern void get_term_info(void);
#ifndef MSDOS
extern boolean tc_tname(FILE * fp, char *term, char *buf);
#endif
extern void tc_gtdata(char *tcf, FILE * fp, char *buf);
extern void tc_gets(char *ibuf, char **tcstr);
extern void tc_gnum(char *ibuf, int *n);
extern void tstp(void);
extern void tc_cmget(void);

#endif /* __CURSES_H__ */
