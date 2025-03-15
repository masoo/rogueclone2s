#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * Ncurses Colors
 */
enum rogue_colors {
	WHITE = 1,
	RED = 2,
	GREEN = 3,
	YELLOW = 4,
	BLUE = 5,
	MAGENTA = 6,
	CYAN = 7,
	WHITE_REVERSE = 8,
	RED_REVERSE = 9,
	GREEN_REVERSE = 10,
	YELLOW_REVERSE = 11,
	BLUE_REVERSE = 12,
	MAGENTA_REVERSE = 13,
	CYAN_REVERSE = 14
};

extern void init_color_attr(void);
extern int get_colorpair_number(utf8_int8_t ch);
extern void set_colorpair_number(utf8_int8_t ch, int num);
extern int addch_rogue(const chtype ch);
extern int mvaddch_rogue(int y, int x, const chtype ch);
extern int addstr_rogue(const char *str);
extern int mvaddstr_rogue(int y, int x, const char *str);
extern chtype mvinch_rogue(int y, int x);

extern int utf8_display_width(const char *str);

extern utf8_int8_t *color_str;

#endif /* not DISPLAY_H */
