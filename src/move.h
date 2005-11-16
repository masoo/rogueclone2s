#ifndef __MOVE_H__
#define __MOVE_H__

extern int can_move(int row1, int col1, int row2, int col2);
extern int is_passable(register int row, register int col);
extern int one_move_rogue(short dirch, short pickup);
extern void rest(int count);
extern void multiple_move_rogue(int dirch);
extern void move_onto(void);
extern int gr_dir(void);
extern int next_to_something(register int drow, register int dcol);
extern boolean is_direction(int c);
extern void heal(void);

#endif /* __MOVE_H__ */
