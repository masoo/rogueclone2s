#ifndef __ROOM_H__
#define __ROOM_H__

extern int get_dungeon_char(register int row, register int col);
extern int get_room_number(register int row, register int col);
extern int is_all_connected(void);
extern void dr_course(object *monster, boolean entering, short row, short col);
extern void gr_row_col(short *row, short *col, unsigned short mask);
extern void light_passage(int row, int col);
extern void light_up_room(int rn);
extern int gr_room(void);
extern int party_objects(int rn);
extern int get_mask_char(register unsigned short mask);
extern void draw_magic_map(void);
extern void darken_room(short rn);
extern void visit_rooms(int rn);
extern int get_oth_room(short rn, short *row, short *col);

#endif /* __ROOM_H__ */
