#ifndef __MACHDEP_H__
#define __MACHDEP_H__

extern void putstr(register char *s);
#ifndef ORIGINAL
extern char *md_getcwd(char *dir, int len);
#endif
extern int md_chdir(char *dir);
extern void md_slurp(void);
extern void md_control_keyboard(boolean mode);
extern void md_heed_signals(void);
extern void md_ignore_signals(void);
extern int md_get_file_id(char *fname);
extern int md_link_count(char *fname);
extern void md_gct(struct rogue_time *rt_buf);
extern void md_gfmt(char *fname, struct rogue_time *rt_buf);
extern boolean md_df(char *fname);
extern char *md_gln(void);
extern void md_sleep(int nsecs);
extern char *md_getenv(char *name);
extern char *md_malloc(int n);
extern int md_gseed(void);
extern void md_exit(int status);
extern void md_cbreak_no_echo_nonl(boolean on);
extern char *md_gdtcf(void);
#ifndef MSDOS
extern void md_tstp(void);
#endif

#endif /* __MACHDEP_H__ */
