#ifndef MOVE_H
#define MOVE_H

extern int one_move_rogue(short dirch, short pickup);
extern void multiple_move_rogue(int dirch);
extern int is_passable(int row, int col);
extern int next_to_something(int drow, int dcol);
extern int can_move(int row1, int col1, int row2, int col2);
extern void move_onto(void);
extern bool is_direction(int c);
extern bool check_hunger(bool messages_only);
extern bool reg_move(void);
extern void rest(int count);
extern int gr_dir(void);
extern void heal(void);

#endif /* not MOVE_H */
