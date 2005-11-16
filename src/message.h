#ifndef __MESSAGE_H__
#define __MESSAGE_H__

extern int r_index(char *str, int ch, boolean last);
extern void check_message(void);
extern void message(char *msg, boolean intrpt);
extern void print_stats(register int stat_mask);

#endif /* __MESSAGE_H__ */
