#ifndef HIT_H
#define HIT_H

typedef struct obj object;

extern void mon_hit(object *monster, char *other, bool flame);
extern void rogue_hit(object *monster, bool force_hit);
extern void rogue_damage(short d, object *monster);
extern int get_damage(char *ds, bool r);
extern int get_w_damage(object *obj);
extern int get_number(char *s);
extern long lget_number(char *s);
extern int to_hit(object *obj);
extern int damage_for_strength(void);
extern int mon_damage(object *monster, int damage);
extern void fight(bool to_the_death);
extern void get_dir_rc(char dir, short *lines, short *columns,
    bool allow_off_screen);
extern int get_hit_chance(object *weapon);
extern int get_weapon_damage(object *weapon);

#endif /* not HIT_H */
