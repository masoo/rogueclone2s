#ifndef __MACHDEP_H__
#define __MACHDEP_H__

extern int md_gseed(void);
extern void md_chdir(char *dir);
extern void md_control_keyboard(boolean mode);
extern void md_exit(int status);
extern void md_heed_signals(void);
extern void md_ignore_signals(void);
extern void putstr(register char *s);

#endif /* __MACHDEP_H__ */
