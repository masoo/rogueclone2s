#ifndef __OBJECT_H__
#define __OBJECT_H__

extern int colored(register int c);
extern int get_armor_class(object *obj);
extern void free_object(object *obj);
extern void free_stuff(object *objlist);
extern void get_food(object *obj, boolean force_ration);
extern void put_amulet(void);
extern void put_objects(void);
extern void put_stairs(void);
extern void make_party(void);
extern int next_party(void);
extern void rand_place(object *obj);
extern void put_gold(void);
extern void plant_gold(short row, short col, boolean is_maze);
extern void place_at(object *obj, int row, int col);
extern object * get_letter_object(int ch);
extern void gr_scroll(object *obj);
extern void gr_potion(object *obj);
extern void gr_weapon(object *obj, int assign_wk);
extern void gr_armor(object *obj, int assign_wk);
extern void gr_wand(object *obj);
extern void show_objects(void);
extern void new_object_for_wizard(void);
extern void list_object(object *obj, short max);

#endif /* __OBJECT_H__ */
