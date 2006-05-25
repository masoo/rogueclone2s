#if !defined( __MACHDEP_H__ )
#define __MACHDEP_H__

/* POSIX �����ƥ�Υ����ʥ�̾ SIGQUIT, SIGHUP �ˤĤ���
 * ���Υޥ���̾�������Ϥ�¸�ߤ��ʤ����� C99 ��ǧ��Ƥ��륷���ʥ�̾
 * �˶����Ѵ�����Ȥ����������ʲ����ˡ
 */
#if !defined( SIGQUIT )
# define SIGQUIT SIGTERM
#endif /* not SIGQUIT */

#if !defined( SIGHUP )
# define SIGHUP SIGABRT
#endif /* not SIGHUP */

#if !defined( HAVE_ASSUME_DEFAULT_COLORS )
# define assume_default_colors(to, from) init_pair(0, to, from)
#endif /* not HAVE_ASSUME_DEFAULT_COLORS */

/* ���Ѥδؿ���Ȥ�ʤ����Ȥǲ��� */
//���Ȥǹͤ��ʤ���
#if !defined( HAVE_ATTR_GET )
# define attr_get
#endif /* not HAVE_ATTR_GET */

#if !defined( HAVE_GETLOGIN )
extern char *getlogin(void);
#endif /* not HAVE_GETLOGIN */

extern void putstr(char *s);
extern int md_chdir(char *dir);
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

#endif /* not __MACHDEP_H__ */
