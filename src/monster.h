#ifndef __MONSTER_H__
#define __MONSTER_H__

extern boolean mon_sees(object *monster, int row, int col);
extern int flit(object *monster);
extern int gmc(object *monster);
extern int gmc_row_col(register int row, register int col);
extern int gr_obj_char(void);
extern int m_confuse(object *monster);
extern int mon_can_go(register object *monster, register short row, register short col);
extern int move_confused(object *monster);
extern int mtry(register object *monster, register short row, register short col);
extern int no_room_for_monster(int rn);
extern int rogue_can_see(register int row, register int col);
extern int rogue_is_around(short row, short col);
extern object *gr_monster(register object *monster, register int mn);
extern void aggravate(void);
extern void aim_monster(object *monster);
extern void create_monster(void);
extern void move_mon_to(register object *monster, register short row, register short col);
extern void mv_aquatars(void);
extern void mv_mons(void);
extern void mv_monster(register object *monster, short row, short col);
extern void party_monsters(int rn, int n);
extern void put_m_at(short row, short col, object *monster);
extern void put_mons(void);
extern void show_monsters(void);
extern void wake_room(short rn, boolean entering, short row, short col);
extern void wake_up(object *monster);
extern void wanderer(void);

#endif /* __MONSTER_H__ */
