/*
 * message.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "utf8.h"
#include "wcwidth.h"

#include "display.h"
#include "init.h"
#include "machdep.h"
#include "message.h"
#include "move.h"
#include "object.h"
#include "pack.h"
#include "rogue.h"

#define CTRL(c) ((c) & 037)

char msg_line[ROGUE_COLUMNS] = "";
short msg_col = 0;
bool msg_cleared = true;
char hunger_str[8] = "";

extern bool cant_int, did_int, interrupted, save_is_interactive;
extern short add_strength;
extern short cur_level;

void
message(char *msg, bool intrpt)
{
	if (!save_is_interactive) {
		return;
	}
	if (intrpt) {
		interrupted = true;
	}
	cant_int = true;

	if (!msg_cleared) {
		mvaddstr_rogue(MIN_ROW - 1, msg_col, mesg[11]);
		refresh();
		wait_for_ack();
		check_message();
	}
	(void)strcpy(msg_line, msg);
	mvaddstr_rogue(MIN_ROW - 1, 0, msg);
	addch_rogue(' ');
	refresh();
	msg_cleared = false;
	msg_col = utf8_display_width(msg);

	cant_int = false;
	if (did_int) {
		did_int = false;
		onintr(
		    0); /* 「0」に意味はないが警告除去のために値を入れる。onintr関数を見直す必要がある。
			 */
	}
}

void
remessage(void)
{
	if (msg_line[0]) {
		message(msg_line, 0);
	}
}

void
check_message(void)
{
	if (msg_cleared) {
		return;
	}
	move(MIN_ROW - 1, 0);
	clrtoeol();
	refresh();
	msg_cleared = true;
}

int
get_direction(void)
{
	int dir;

	message(mesg[55], 0);
	while (!is_direction(dir = rgetchar())) {
		sound_bell();
	}
	check_message();
	return dir;
}

int
get_input_line(char *prompt, char *insert, char *buf, char *if_cancelled,
    bool add_blank, bool do_echo)
{
	int n;

	n = do_input_line(1, 0, 0, prompt, insert, buf, if_cancelled, add_blank,
	    do_echo, 0);
	return ((n < 0) ? 0 : n);
}

int
input_line(int row, int col, char *insert, char *buf, int ch)
{
	return do_input_line(0, row, col, "", insert, buf, "", 0, 1, ch);
}

int
do_input_line(bool is_msg, int row, int col, char *prompt, char *insert,
    char *buf, char *if_cancelled, bool add_blank, bool do_echo, int first_ch)
{
	short ch;
	short i = 0, n = 0;
	short cp_bytes[MAX_TITLE_LENGTH]; /* 各コードポイントのバイト数 */
	short cp_width[MAX_TITLE_LENGTH]; /* 各コードポイントの表示幅 */
	short ncp = 0;         /* 入力済みコードポイント数 */
	short display_col = 0; /* 現在の表示カラム位置 */

	if (is_msg) {
		message(prompt, 0);
		n = utf8_display_width(prompt) + 1;
	} else {
		mvaddstr_rogue(row, col, prompt);
	}

	if (insert[0]) {
		mvaddstr_rogue(row, col + n, insert);
		(void)strcpy(buf, insert);
		i = strlen(insert);
		/* 既存テキストのコードポイント情報を記録する */
		ncp = 0;
		display_col = 0;
		short k = 0;
		while (k < i) {
			size_t cpsize = utf8codepointcalcsize(&buf[k]);
			utf8_int32_t cp;
			utf8codepoint(&buf[k], &cp);
			int w = (cp < 0x80) ? 1 : wcwidth((wchar_t)cp);
			if (w < 0)
				w = 0;
			cp_bytes[ncp] = (short)cpsize;
			cp_width[ncp] = (short)w;
			display_col += w;
			ncp++;
			k += cpsize;
		}
		move(row, col + n + display_col);
		refresh();
	}
	for (;;) {
		if (first_ch) {
			ch = first_ch;
			first_ch = 0;
		} else {
			ch = rgetchar();
		}
		if (ch == '\r' || ch == '\n' || ch == CANCEL) {
			break;
		}
		if ((ch == '\b') && (ncp > 0)) {
			ncp--;
			i -= cp_bytes[ncp];
			display_col -= cp_width[ncp];
			if (do_echo) {
				mvaddstr_rogue(row, col + n + display_col,
				    cp_width[ncp] == 2 ? "  " : " ");
				move(row, col + n + display_col);
			}
		} else if (ch >= ' ' && ch < 0x80 && i < MAX_TITLE_LENGTH - 2) {
			/* ASCII 印字可能文字 */
			if ((ch != ' ') || (i > 0)) {
				buf[i] = ch;
				cp_bytes[ncp] = 1;
				cp_width[ncp] = 1;
				ncp++;
				display_col++;
				if (do_echo) {
					addch(ch);
				}
				i++;
			}
		} else if ((ch & 0xC0) == 0xC0 && i < MAX_TITLE_LENGTH - 5) {
			/* UTF-8 マルチバイト文字の先頭バイト */
			size_t expected;
			if ((ch & 0xF8) == 0xF0)
				expected = 4;
			else if ((ch & 0xF0) == 0xE0)
				expected = 3;
			else
				expected = 2;

			buf[i] = ch;
			for (size_t b = 1; b < expected; b++) {
				buf[i + b] = rgetchar();
			}

			utf8_int32_t cp;
			utf8codepoint(&buf[i], &cp);
			int w = wcwidth((wchar_t)cp);
			if (w < 0)
				w = 0;
			cp_bytes[ncp] = (short)expected;
			cp_width[ncp] = (short)w;
			ncp++;
			display_col += w;

			if (do_echo) {
				char tmp[5];
				memcpy(tmp, &buf[i], expected);
				tmp[expected] = '\0';
				addstr(tmp);
			}
			i += expected;
		}
		refresh();
	}
	if (is_msg) {
		check_message();
	}
	/* 末尾のスペースを除去する */
	while ((ncp > 0) && (buf[i - 1] == ' ') && (cp_bytes[ncp - 1] == 1)) {
		ncp--;
		i--;
	}
	if (add_blank) {
		buf[i++] = ' ';
	}
	buf[i] = 0;

	if ((ch == CANCEL) || (i == 0) || ((i == 1) && add_blank)) {
		if (is_msg && if_cancelled) {
			message(if_cancelled, 0);
		}
		return ((ch == CANCEL) ? -1 : 0);
	}
	return i;
}

int
rgetchar(void)
{
	int ch;

	for (;;) {
		ch = getch();

		switch (ch) {
		case '\022':
			wrefresh(curscr);
			break;
			/*
			 * can't use X for save_screen purpose
			 * because 2nd byte of kanji might be an 'X'!
			 */
		case CTRL('D'):
			save_screen();
			break;
		default:
			return ch;
		}
	}
}

/*
Level: 99 Gold: 999999 Hp: 999(999) Str: 99(99) Arm: 99 Exp: 21/10000000 Hungry
階: 99 金塊: 999999 体力: 999(999) 強さ: 99(99) 守備: 99 経験: 21/10000000 空腹
0    5    1    5    2    5    3    5    4    5    5    5    6    5    7    5
*/

void
print_stats(int stat_mask)
{
	char buf[16];
	bool label;
	int row = ROGUE_LINES - 1;

	label = (stat_mask & STAT_LABEL) ? 1 : 0;

	if (stat_mask & STAT_LEVEL) {
		if (label) {
			mvaddstr_rogue(row, 0, mesg[56]);
		}
		/* max level taken care of in make_level() */
		sprintf(buf, "%d", cur_level);
		mvaddstr_rogue(row, 4, buf);
		pad(buf, 2);
	}
	if (stat_mask & STAT_GOLD) {
		if (label) {
			if (rogue.gold > MAX_GOLD) {
				rogue.gold = MAX_GOLD;
			}
			mvaddstr_rogue(row, 7, mesg[57]);
		}
		sprintf(buf, "%ld", rogue.gold);
		mvaddstr_rogue(row, 13, buf);
		pad(buf, 6);
	}
	if (stat_mask & STAT_HP) {
		if (label) {
			mvaddstr_rogue(row, 20, mesg[58]);
			if (rogue.hp_max > MAX_HP) {
				rogue.hp_current -= (rogue.hp_max - MAX_HP);
				rogue.hp_max = MAX_HP;
			}
		}
		sprintf(buf, "%d(%d)", rogue.hp_current, rogue.hp_max);
		mvaddstr_rogue(row, 26, buf);
		pad(buf, 8);
	}
	if (stat_mask & STAT_STRENGTH) {
		if (label) {
			mvaddstr_rogue(row, 35, mesg[59]);
		}
		if (rogue.str_max > MAX_STRENGTH) {
			rogue.str_current -= (rogue.str_max - MAX_STRENGTH);
			rogue.str_max = MAX_STRENGTH;
		}
		sprintf(buf, "%d(%d)", (rogue.str_current + add_strength),
		    rogue.str_max);
		mvaddstr_rogue(row, 41, buf);
		pad(buf, 6);
	}
	if (stat_mask & STAT_ARMOR) {
		if (label) {
			mvaddstr_rogue(row, 48, mesg[60]);
		}
		if (rogue.armor && (rogue.armor->d_enchant > MAX_ARMOR)) {
			rogue.armor->d_enchant = MAX_ARMOR;
		}
		sprintf(buf, "%d", get_armor_class(rogue.armor));
		mvaddstr_rogue(row, 54, buf);
		pad(buf, 2);
	}
	if (stat_mask & STAT_EXP) {
		if (label) {
			mvaddstr_rogue(row, 57, mesg[61]);
		}
		/*  Max exp taken care of in add_exp() */
		sprintf(buf, "%d/%ld", rogue.exp, rogue.exp_points);
		mvaddstr_rogue(row, 63, buf);
		pad(buf, 11);
	}
	if (stat_mask & STAT_HUNGER) {
		mvaddstr_rogue(row, 75, hunger_str);
		clrtoeol();
	}
	refresh();
}

void
pad(char *s, short n)
{
	short i;

	for (i = strlen(s); i < n; i++) {
		addch_rogue(' ');
	}
}

void
save_screen(void)
{
	FILE *fp;
	short i, j;
	char buf[ROGUE_COLUMNS + 2];
	bool found_non_blank;

	if ((fp = fopen("rogue.screen", "w")) != NULL) {
		for (i = 0; i < ROGUE_LINES; i++) {
			found_non_blank = false;
			for (j = (ROGUE_COLUMNS - 1); j >= 0; j--) {
				buf[j] = mvinch_rogue(i, j);
				if (!found_non_blank) {
					if ((buf[j] != ' ') || (j == 0)) {
						buf[j + ((j == 0) ? 0 : 1)] = 0;
						found_non_blank = true;
					}
				}
			}
			fputs(buf, fp);
			putc('\n', fp);
		}
		fclose(fp);
	} else {
		sound_bell();
	}
}

void
sound_bell(void)
{
	putchar(7);
	fflush(stdout);
}

bool
is_digit(short ch)
{
	return (bool)((ch >= '0') && (ch <= '9'));
}

int
r_index(char *str, int ch, bool last)
{
	int i;

	if (last) {
		for (i = strlen(str) - 1; i >= 0; i--) {
			if (str[i] == ch) {
				return i;
			}
		}
	} else {
		for (i = 0; str[i]; i++) {
			if (str[i] == ch) {
				return i;
			}
		}
	}
	return -1;
}
