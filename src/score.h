#ifndef __SCORE_H__
#define __SCORE_H__

extern void killed_by(object *monster, short other);
extern void put_scores(object * monster, short other);
extern void quit(boolean from_intrpt);
extern void win(void);
extern void xxxx(char *buf, short n);
extern void center(short row, char *buf);
extern void mvaddbanner(int row, int col, register int *ban);
extern void id_all(void);
extern void sell_pack(void);
extern void sf_error(void);
extern void insert_score(char scores[][82], char n_names[][30], char *n_name, short rank, short n, object *monster, int other);
extern void nickize(char *buf, char *score, char *n_name);
extern int get_value(object *obj);

#endif /* __SCORE_H__ */
