#ifndef __PACK_H__
#define __PACK_H__

extern void do_wear(object *obj);
extern void do_wield(object *obj);
extern void take_from_pack(object *obj, object *pack);
extern void wait_for_ack(void);
extern int pack_count(object *new_obj);
extern void drop(void);
extern void take_off(void);
extern void wear(void);
extern void wield(void);
extern void call_it(void);
extern void kick_into_pack(void);
extern int pack_letter(char *prompt, unsigned short mask);
extern object *add_to_pack(object *obj, object *pack, int condense);
extern int next_avail_ichar(void);
extern object *pick_up(int row, int col, short *status);
extern void unwield(object *obj);
extern void unwear(object *obj);
extern int is_pack_letter(short *c, unsigned short *mask);

#endif /* __PACK_H__ */
