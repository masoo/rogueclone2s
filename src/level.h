#ifndef __LEVEL_H__
#define __LEVEL_H__

extern void make_level(void);
extern void make_room(short rn, short r1, short r2, short r3);
extern int connect_rooms(register short room1, register short room2);
extern void clear_level(void);
extern void put_door(room * rm, short dir, register short *row, register short *col);
extern void draw_simple_passage(short row1, short col1, short row2, short col2, short dir);
extern int same_row(int room1, int room2);
extern int same_col(int room1, int room2);
extern void add_mazes(void);
extern void fill_out_level(void);
extern void fill_it(register int rn, boolean do_rec_de);
extern void recursive_deadend(short rn, short *offsets, short srow, short scol);
extern boolean mask_room(short rn, short *row, short *col, unsigned short mask);
extern void make_maze(register short r, register short c, register short tr, register short br, register short lc, register short rc);
extern void hide_boxed_passage(short row1, short col1, short row2, short col2, short n);
extern void put_player(short nr);
extern int drop_check(void);
extern int check_up(void);
extern void add_exp(int e, boolean promotion);
extern int get_exp_level(long e);
extern int hp_raise(void);
extern void show_average_hp(void);
extern void mix_random_rooms(void);

#endif /* __LEVEL_H__ */