#ifndef __THROW_H__
#define __THROW_H__

extern void rand_around(short i, short  *r, short *c);
extern void throw(void);
extern int throw_at_monster(object *monster, object *weapon);
extern void flop_weapon(object *weapon, short row, short col);
extern void potion_monster(object *monster, unsigned short kind);

#endif /* __THROW_H__ */
