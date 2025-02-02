#ifndef SAVE_H
#define SAVE_H

extern void save_game(void);
extern void save_into_file(char *sfile);
extern void restore(char *fname);
extern void write_pack(object *pack, FILE *fp);
extern void read_pack(object *pack, FILE *fp, bool is_rogue);
extern void rw_dungeon(FILE *fp, bool rw);
extern void rw_id(struct id id_table[], FILE *fp, int n, bool wr);
extern void write_string(char *s, FILE *fp);
extern void read_string(char *s, FILE *fp);
extern void rw_rooms(FILE *fp, bool rw);
extern void r_read(FILE *fp, char *buf, int n);
extern void r_write(FILE *fp, char *buf, int n);
extern bool has_been_touched(struct rogue_time *saved_time,
				struct rogue_time *mod_time);

#endif /* not SAVE_H */
