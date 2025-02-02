#ifndef TRAP_H
#define TRAP_H

extern int trap_at(int row, int col);
extern void trap_player(short row, short col);
extern void add_traps(void);
extern void id_trap(void);
extern void show_traps(void);
extern void search(short n, bool is_auto);

#endif /* not TRAP_H */
