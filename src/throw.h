#ifndef THROW_H
#define THROW_H

extern void throw(void);
extern int throw_at_monster(object *monster, object *weapon);
extern object *get_thrown_at_monster(object *obj, short dir, short *row,
				     short *col);
extern void flop_weapon(object *weapon, short row, short col);
extern void rand_around(short i, short *r, short *c);
extern void potion_monster(object *monster, unsigned short kind);

#endif /* not THROW_H */
