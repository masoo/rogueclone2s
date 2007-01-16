#ifndef INIT_H
#define INIT_H

extern int init(int argc, char *argv[]);
extern void player_init(void);
extern void clean_up(char *estr);
extern void start_window(void);
extern void stop_window(void);
extern void byebye(int sig);
extern void onintr(int sig);
extern void error_save(int sig);
extern void do_args(int argc, char *argv[]);
extern void do_opts(void);
extern void set_opts(char *env);
extern void env_get_value(char **s, char *e, bool add_blank,
			  bool no_colon);

#endif /* not INIT_H */
