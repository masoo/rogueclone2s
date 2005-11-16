#ifndef __SAVE_H__
#define __SAVE_H__

extern void restore(char *fname);
extern void save_into_file(char *sfile);
extern void save_game(void);
extern void r_write(FILE *fp, char *buf, int n);
extern void write_string(char *s, FILE *fp);
extern void write_pack(object *pack, FILE *fp);
extern void rw_dungeon(FILE *fp, boolean rw);
extern void rw_id(struct id id_table[], FILE *fp, int n, boolean wr);
extern void rw_rooms(FILE *fp, boolean rw);
extern void r_read(FILE *fp, char *buf, int n);
extern void read_string(char *s, FILE *fp);
extern void read_pack(object *pack, FILE *fp, boolean is_rogue);

#endif /* __SAVE_H__ */
