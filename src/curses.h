#ifndef __CURSES_H__
#define __CURSES_H__

extern void clrtoeol(void);
extern void crmode(void);
extern void endwin(void);
extern void initscr(void);
extern void move(short row, short col);
extern void noecho(void);
extern void nonl(void);
extern void refresh(void);
extern void repaint_screen(void);

#endif /* __CURSES_H__ */
