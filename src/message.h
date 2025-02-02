#ifndef MESSAGE_H
#define MESSAGE_H

extern void message(char *msg, bool intrpt);
extern void remessage(void);
extern void check_message(void);
extern int get_direction(void);
extern int get_input_line(char *prompt, char *insert, char *buf,
			  char *if_cancelled, bool add_blank,
			  bool do_echo);
extern int input_line(int row, int col, char *insert, char *buf, int ch);
extern int do_input_line(bool is_msg, int row, int col, char *prompt,
			 char *insert, char *buf, char *if_cancelled,
			 bool add_blank, bool do_echo, int first_ch);
extern int rgetchar(void);
extern void print_stats(int stat_mask);
extern void pad(char *s, short n);
extern void save_screen(void);
extern void sound_bell(void);
extern bool is_digit(short ch);
extern int r_index(char *str, int ch, bool last);

#endif /* not MESSAGE_H */
