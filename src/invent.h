#ifndef __INVENT_H__
#define __INVENT_H__

extern void get_wand_and_ring_materials(void);
extern void make_scroll_titles(void);
extern void mix_colors(void);
extern void znum(char *buf, int n, int plus);
extern void get_desc(register object *obj, register char *desc, boolean capitalized);
extern void inventory(object *pack, unsigned short maskd);
extern void inv_armor_weapon(boolean is_weapon);
extern void single_inv(short ichar);
extern void discovered(void);
extern void lznum(char *buf, long n, int plus);

#endif /* __INVENT_H__ */
