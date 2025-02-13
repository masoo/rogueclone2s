#ifndef INVENT_H
#define INVENT_H

typedef struct obj object;
typedef struct sid id;

extern void inventory(object *pack, unsigned short maskd);
extern void mix_colors(void);
extern void make_scroll_titles(void);
extern void get_desc(object *obj, char *desc, bool capitalized);
extern void get_wand_and_ring_materials(void);
extern void single_inv(short ichar);
extern id *get_id_table(object *obj);
extern void inv_armor_weapon(bool is_weapon);
extern void discovered(void);
extern void znum(char *buf, int n, int plus);

#endif /* not INVENT_H */
