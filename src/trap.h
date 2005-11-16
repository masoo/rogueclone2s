#ifndef __TRAP_H__
#define __TRAP_H__

extern void add_traps(void);
extern void search(short n, boolean is_auto);
extern void id_trap(void);
extern void show_traps(void);
extern int trap_at(register int row, register int col);
extern void trap_player(short row, short col);

#endif /* __TRAP_H__ */
