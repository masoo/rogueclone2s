#ifndef __MACHDEP_H__
#define __MACHDEP_H__

extern int md_gseed(void);
extern int md_chdir(char *dir);
extern void md_control_keyboard(boolean mode);
extern void md_exit(int status);
extern void md_heed_signals(void);
extern void md_ignore_signals(void);
extern void md_slurp(void);
extern void putstr(register char *s);
extern void md_cbreak_no_echo_nonl(boolean on);
extern void md_gct(struct rogue_time *rt_buf);
extern void md_gfmt(char *fname, struct rogue_time *rt_buf);
extern void md_sleep(int nsecs);
extern int md_get_file_id(char *fname);
extern int md_link_count(char *fname);
extern void md_tstp(void);


#endif /* __MACHDEP_H__ */
