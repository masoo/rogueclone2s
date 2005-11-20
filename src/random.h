#ifndef __RANDOM_H__
#define __RANDOM_H__

extern void srrandom(int x);
extern long rrandom(void);
extern int get_rand(register int x, register int y);
extern int rand_percent(register int percentage);
extern int coin_toss(void);

#endif /* __RANDOM_H__ */
