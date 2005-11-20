#ifndef __MONSTER_H__
#define __MONSTER_H__

extern void put_mons(void);
extern object *gr_monster(register object *monster, register int mn);
extern void mv_mons(void);
extern void party_monsters(int rn, int n);
extern int gmc_row_col(register int row, register int col);
extern int gmc(object *monster);
extern void mv_monster(register object *monster, short row, short col);
extern int mtry(register object *monster, register short row, register short col);
extern void move_mon_to(register object *monster, register short row, register short col);
extern int mon_can_go(register object *monster, register short row, register short col);
extern void wake_up(object *monster);
extern void wake_room(short rn, boolean entering, short row, short col);
extern char *mon_name(object *monster);
extern int rogue_is_around(short row, short col);
extern void wanderer(void);
extern void show_monsters(void);
extern void create_monster(void);
extern void put_m_at(short row, short col, object *monster);
extern void aim_monster(object *monster);
extern int rogue_can_see(register int row, register int col);
extern int move_confused(object *monster);
extern int flit(object *monster);
extern int gr_obj_char(void);
extern int no_room_for_monster(int rn);
extern void aggravate(void);
extern boolean mon_sees(object *monster, int row, int col);
extern void mv_aquatars(void);

#endif /* __MONSTER_H__ */
