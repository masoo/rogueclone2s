/*
 * curses.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#ifdef CURSES

/* The following is a curses emulation package suitable for the rogue program
 * in which it is included.  No other suitability is claimed or suspected.
 * Only those routines currently needed by this rogue program are included.
 * This is being provided for those systems that don't have a suitable
 * curses package and want to run this rogue program.
 *
 * Compile the entire program with -DCURSES to incorporate this package.
 *
 * The following is NOT supported:
 *   "%D", "%B", "%n", or "%>" inside a cursor motion (cm) termcap string.
 *   Terminals in which the cursor motion addresses the row differently from
 *       the column, as in ":cm=\E%2,%3" or ":cm=\EY%+x;%+y"
 *   Termcap database stored in the TERMCAP environ variable as returned
 *       from md_getenv().  Only the termcap file name can be stored there.
 *       See the comments for md_getenv() in machdep.c.
 *   Terminals without non-destructive backspace.  Backspace (^H) is used
 *       for cursor motion regardless of any termcap entries.
 *   The ":tc=" termcap entry is ignored.
 *
 * Suggestions:
 *   Use line-feed as your termcap "do" entry: ":do=^J", ":do=\012" or
 *      ":do=\n"  This will help cursor motion optimization.  If line-feed
 *      won't work, then a short escape sequence will do.  Same goes for "up"
 */

#include <stdio.h>
#include "rogue.h"

#ifndef MSDOS
boolean tc_tname();
#endif

#define BS 010
#define LF 012
#define CR 015
#define ESC '\033'
#define TAB '\011'

#ifdef COLOR
#define STANDOUT  0x0800
#define BASECOLOR 0x0700
#define ALLCOLOR  0x0f00
#define NOCOLOR   0x00ff
#else
#ifdef JAPAN
#define ST_MASK 0x100	/* by Yasha */
#else
#define ST_MASK 0x80
#endif	/* JAPAN */
#endif	/* COLOR */
#define BUFLEN 256

/*#ifdef COLOR*/
#if defined(COLOR) || defined(JAPAN)	/* short when kanji by Yasha */
short terminal[DROWS][DCOLS];
short buffer[DROWS][DCOLS];
#else
char terminal[DROWS][DCOLS];
char buffer[DROWS][DCOLS];
#endif
#ifndef MSDOS
char *tc_file;
#endif

#ifndef MSDOS
char cm_esc[16], cm_sep[16], cm_end[16];
boolean cm_reverse = 0;
boolean cm_two = 0;
boolean cm_three = 0;
boolean cm_char = 0;
short cm_inc = 0;
#endif

boolean screen_dirty;
boolean lines_dirty[DROWS];
#ifdef COLOR
short buf_attr = 0;
short term_attr = 0;
extern boolean do_color;
#else
boolean buf_stand_out = 0;
boolean term_stand_out = 0;
#endif

int LINES = DROWS, COLS = DCOLS;
WINDOW scr_buf;
WINDOW *curscr = &scr_buf;

#ifdef MSDOS
char *SO = "\033[7m";
char *SE = "\033[m";
#else
char *CL = (char *) 0;
char *CM = (char *) 0;
char *UC = (char *) 0;
char *DO = (char *) 0;
char *VS = "";
char *VE = "";
char *TI = "";
char *TE = "";
char *SO = "";
char *SE = "";
char *TC = (char *) 0;		/* by Yasha */
#endif

short cur_row, cur_col;

initscr()
{
#ifndef MSDOS
	get_term_info();
#endif
	clear();
#ifdef MSDOS
	putstr(cursor_off);
#else
	putstr(TI);
	putstr(VS);
#endif
}

endwin()
{
#ifdef COLOR
	putstr("\033[m");
	term_attr = 0;
#endif
#ifdef MSDOS
	putstr(cursor_on);
#else
	putstr(TE);
	putstr(VE);
#endif
	md_cbreak_no_echo_nonl(0);
}

#ifdef COLOR
repaint_screen()
{
	register col, row;
	short ch;

	for (row = MIN_ROW; row < DROWS - 1; row++)
		for (col = 0; col < DCOLS; col++)
			if ((ch = mvinch(row, col)) != ' ')
				mvaddch(row, col, colored(ch));
	refresh();
}
#endif

move(row, col)
short row, col;
{
	curscr->_cury = row;
	curscr->_curx = col;
	screen_dirty = 1;
}

mvaddstr(row, col, str)
short row, col;
char *str;
{
	move(row, col);
	addstr(str);
}

addstr(str)
register char *str;
{
	while (*str) {
/*		addch((int) *str++);*/
		addch(((unsigned int) *str++) & 0xff);
	}
}

#ifdef COLOR
addch(ch)
register int ch;
{
	register short row, col;

	if (ch == '\t') {
		col = curscr->_curx % 8;
		while (col++ != 8)
			addch(' ');
		return;
	}
	row = curscr->_cury;
	col = curscr->_curx++;
	if (buf_attr) {
		ch |= buf_attr;
	}
	if (!do_color) {
		ch &= ~BASECOLOR;
	}
	buffer[row][col] = (short) ch;
	lines_dirty[row] = 1;
	screen_dirty = 1;
}
#else
addch(ch)
register int ch;
{
	register short row, col;

	if (ch == '\t') {
		col = curscr->_curx % 8;
		while (col++ != 8)
			addch(' ');
		return;
	}
	row = curscr->_cury;
	col = curscr->_curx++;
	if (buf_stand_out) {
		ch |= ST_MASK;
	}
#ifdef JAPAN
	buffer[row][col] = (short) ch;	/* by Yasha */
#else
	buffer[row][col] = (char) ch;
#endif
	lines_dirty[row] = 1;
	screen_dirty = 1;
}
#endif

mvaddch(row, col, ch)
short row, col;
int ch;
{
	move(row, col);
	addch(ch);
}

#ifdef JAPAN
touch(row, bcol, ecol)
register int row;
{
	register int i;

	for (i = bcol; i <= ecol; i++)
		terminal[row][i] = 0;
	lines_dirty[row] = 1;
}
#endif /*JAPAN*/

refresh()
{
	int i, j;
	short old_row, old_col;
/*#ifdef COLOR*/
#if defined(COLOR) || defined(JAPAN)	/* short when kanji by Yasha */
	register short *bp, *tp;
#else
	register char *bp, *tp;
#endif

	if (screen_dirty) {
		old_row = curscr->_cury;
		old_col = curscr->_curx;
		for (i = 0; i < DROWS; i++) {
			if (lines_dirty[i]) {
				bp = buffer[i];
				tp = terminal[i];
				for (j = 0; j < DCOLS; j++) {
					if (*bp != *tp) {
						if (j!=cur_col || i!=cur_row)
							put_cursor(i, j);
						put_st_char(*tp++ = *bp++);
						cur_col++;
					} else {
						bp++;
						tp++;
					}
				}
				lines_dirty[i] = 0;
			}
		}
		put_cursor(old_row, old_col);
		screen_dirty = 0;
		fflush(stdout);
	}
}

wrefresh(scr)
WINDOW *scr;
{
	short i;
	register short col;
/*#ifdef COLOR*/
#if defined(COLOR) || defined(JAPAN)	/* short when kanji by Yasha */
	register short *p;
#else
	register char *p;
#endif

#ifdef MSDOS
	putstr("\033[2J");	/* clear screen */
#else
	putstr(CL);
#endif
	cur_row = cur_col = 0;

	for (i = 0; i < DROWS; i++) {
		p = buffer[i];
		col = 0;
		while (col < DCOLS) {
			while ((col < DCOLS) && (p[col] == ' ')) {
				col++;
			}
			if (col < DCOLS) {
				put_cursor(i, col);
			}
			while ((col < DCOLS) && (p[col] != ' ')) {
/*				put_st_char((int) p[col++]);*/
				put_st_char((unsigned char) p[col++]);
				cur_col++;
			}
		}
	}
	put_cursor(curscr->_cury, curscr->_curx);
	fflush(stdout);
#ifndef MSDOS
	scr = scr;		/* make lint happy */
#endif
}

mvinch(row, col)
register short row, col;
{
	move(row, col);
#ifdef COLOR
	return((int) buffer[row][col] & NOCOLOR);
#else
	return((int) buffer[row][col]);
#endif
}

clear()
{
#ifdef MSDOS
	putstr("\033[2J");	/* clear screen */
#else
	putstr(CL);
#endif
	fflush(stdout);
	cur_row = cur_col = 0;
	move(0, 0);
	clear_buffers();
}

clrtoeol()
{
	register short row, col;

	row = curscr->_cury;

	for (col = curscr->_curx; col < DCOLS; col++) {
		buffer[row][col] = ' ';
	}
	lines_dirty[row] = 1;
}

#ifdef COLOR
color(col)
short col;
{
	buf_attr = (col << 8);
}

standout()
{
	buf_attr |= STANDOUT;
}

standend()
{
	buf_attr = 0;
}
#else
standout()
{
	buf_stand_out = 1;
}

standend()
{
	buf_stand_out = 0;
}
#endif

#ifndef MSDOS
crmode()
{
	md_cbreak_no_echo_nonl(1);
}
#endif

#ifndef MSDOS
noecho()
{
	/* crmode() takes care of this */
}
#endif

#ifndef MSDOS
nonl()
{
	/* crmode() takes care of this */
}
#endif

clear_buffers()
{
	int i, j;
/*#ifdef COLOR*/
#if defined(COLOR) || defined(JAPAN)	/* short when kanji by Yasha */
	register short *bufp, *termp;
#else
	register char *bufp, *termp;
#endif

	screen_dirty = 0;

	for (i = 0; i < DROWS; i++) {
		lines_dirty[i] = 0;
		bufp = buffer[i];
		termp = terminal[i];
		for (j = 0; j < DCOLS; j++) {
			*bufp++ = *termp++ = ' ';
		}
	}
}

#ifdef MSDOS
put_cursor(row, col)
short row, col;
{
	register char *p;
	char buf[10];

	if (row == cur_row && col == cur_col)
		return;

	cur_row = row++;
	cur_col = col++;
	p = buf;
	*p++ = ESC;
	*p++ = '[';
	if (row >= 10)
		*p++ = row / 10 + '0';
	*p++ = row % 10 + '0';
	*p++ = ';';
	if (col >= 10)
		*p++ = col / 10 + '0';
	*p++ = col % 10 + '0';
	*p++ = 'H';
	*p = 0;
	putstr(buf);
}
#else
put_cursor(row, col)
register row, col;
{
	register i, rdif, cdif;
	short ch, t;

	rdif = (row > cur_row) ? row - cur_row : cur_row - row;
	cdif = (col > cur_col) ? col - cur_col : cur_col - col;

	if (((row > cur_row) && DO) || ((cur_row > row) && UC)) {
		if ((rdif < 4) && (cdif < 4)) {
			for (i = 0; i < rdif; i++) {
				putstr((row < cur_row) ? UC : DO);
			}
			cur_row = row;
			if (col == cur_col) {
				return;
			}
		}
	}
	if (row == cur_row) {
		if (cdif <= 6) {
		for (i = 0; i < cdif; i++) {
				ch = (col < cur_col) ? BS :
						terminal[row][cur_col + i];
				put_st_char((int) ch);
			}
			cur_row = row;
			cur_col = col;
			return;
		}
	}
	cur_row = row;
	cur_col = col;

	row += cm_inc;
	col += cm_inc;

	if (cm_reverse) {
		t = row;
		row = col;
		col = t;
	}
	putstr(cm_esc);
	if (cm_two) {
		putchar(row / 10 + '0');
		putchar(row % 10 + '0');
		putstr(cm_sep);
		putchar(col / 10 + '0');
		putchar(col % 10 + '0');
	} else if (cm_three) {
		putchar(row / 100 + '0');
		putchar(row / 10 % 10 + '0');
		putchar(row % 10 + '0');
		putstr(cm_sep);
		putchar(col / 100 + '0');
		putchar(col / 10 % 10 + '0');
		putchar(col % 10 + '0');
	} else if (cm_char) {
		putchar(row);
		putstr(cm_sep);
		putchar(col);
	} else {
		if (row >= 10)
			putchar(row / 10 + '0');
		putchar(row % 10 + '0');
		putstr(cm_sep);
		if (col >= 10)
			putchar(col / 10 + '0');
		putchar(col % 10 + '0');
	}
	putstr(cm_end);
}
#endif

#ifdef COLOR
put_st_char(ch)
register ch;
{
	register m;
#ifdef HUMAN
	static char *cstr[] = {
		"\033[m",     "\033[32m",   "\033[32m",   "\033[36m",
		"\033[31m",   "\033[37m",   "\033[31m",   "",
		"\033[7m",    "\033[32;7m", "\033[32;7m", "\033[36;7m",
		"\033[31;7m", "\033[37;7m", "\033[31;7m"
	};
#else
	static char *cstr[] = {
		"\033[m",     "\033[31m",   "\033[32m",   "\033[33m",
		"\033[34m",   "\033[35m",   "\033[36m",   "",
		"\033[7m",    "\033[31;7m", "\033[32;7m", "\033[33;7m",
		"\033[34;7m", "\033[35;7m", "\033[36;7m"
	};
#endif

	if ((m = ch & ALLCOLOR) != term_attr) {
		putstr(cstr[m >> 8]);
		term_attr = ch & ALLCOLOR;
	}
	putchar(ch & NOCOLOR);
}
#else	/* COLOR */

#ifdef JAPAN		/* by Yasha (till "#else" of JAPAN) */
put_st_char(ch)
register short ch;
{
	static char wchrbuf[3] = "\0\0";
	static boolean kanji_flg = 0;

	if ((ch & ST_MASK) && (!term_stand_out)) {
		ch &= ~ST_MASK;
		putstr(SO);
		term_stand_out = 1;
	} else if ((!(ch & ST_MASK)) && term_stand_out) {
		putstr(SE);
		term_stand_out = 0;
	} else {
		ch &= ~ST_MASK;
	}

	if (kanji_flg) {
		wchrbuf[1] = (char) ch;
		putstr(wchrbuf);
		kanji_flg = 0;
#ifdef EUC
	} else if (ch & 0x80) {
#else	/* Shift JIS */
	} else if ((ch>=0x81 && ch<=0x9f) || (ch>=0xe0 && ch<=0xfc)) {
#endif
		wchrbuf[0] = (char) ch;
		kanji_flg = 1;
	} else
		putchar(ch);
}
#else	/* JAPAN */
put_st_char(ch)
register ch;
{
	if ((ch & ST_MASK) && (!term_stand_out)) {
		ch &= ~ST_MASK;
		putstr(SO);
		putchar(ch);
		term_stand_out = 1;
	} else if ((!(ch & ST_MASK)) && term_stand_out) {
		putstr(SE);
		putchar(ch);
		term_stand_out = 0;
	} else {
		ch &= ~ST_MASK;
		putchar(ch);
	}
}
#endif	/* JAPAN */
#endif

#ifndef MSDOS
get_term_info()
{
	FILE *fp;
	char *term, *tcf;
	char buf[BUFLEN];

/* rewritten by Yasha */
	if (tcf = md_getenv("TERMCAP")) {
		if (strlen(tcf) > 40
		    || (fp = fopen(tcf, "r")) == NULL) {
			tc_gtdata(tcf, (FILE *) NULL, buf);
			goto get_term_end;
		} else {
			fclose(fp);
			tc_file = tcf;
		}
	} else
		if (!(tc_file = tcf ? tcf :  md_gdtcf()))
			clean_up("I need a termcap file");

	if (!(term = md_getenv("TERM"))) {
		clean_up("Cannot find TERM variable in environ");
	}

	do {
	  TC = (char *) 0;
	  if ((fp = fopen(tc_file, "r")) == NULL) {
		sprintf(buf, "Cannot open TERMCAP file: %s", tc_file);
		clean_up(buf);
	  }
	  if (!tc_tname(fp, term, buf)) {
	    sprintf(buf, "Cannot find TERM type: %s in TERMCAP file: %s", term,
		    tc_file);
	    clean_up(buf);
	  }
	  tc_gtdata(NULL, fp, buf);	/* by Yasha */
	  fclose(fp);
	} while (term = TC);

    get_term_end:
	if ((!CM) || (!CL)) {
		clean_up("Terminal and termcap must have cm and cl");
	}
	tc_cmget();
/* end of patch */
}
#endif

#ifndef MSDOS
boolean
tc_tname(fp, term, buf)
FILE *fp;
char *term;
char *buf;
{
	short i, j;
	boolean found = 0;
	char *fg;

	while (!found) {
		i = 0;
		fg = fgets(buf, BUFLEN, fp);
		if (fg != NULL) {
			if (	(buf[0] != '#') && (buf[0] != ' ') && (buf[0] != TAB) &&
					(buf[0] != CR) && (buf[0] != LF)) {
				while (buf[i] && (!found)) {
					j = 0;
					while (buf[i] == term[j]) {
						i++;
						j++;
					}
					if ((!term[j]) && ((buf[i] == '|') || (buf[i] == ':'))) {
						found = 1;
					} else {
						while (buf[i] && (buf[i] != '|') && (buf[i] != ':')) {
							i++;
						}
						if (buf[i]) {
							i++;
						}
					}
				}
			}
		} else {
			break;
		}
	}
	return(found);
}
#endif

/* Note (by Yasha)
	parameter 'tcf' is termcap string (if not NULL) */

#ifndef MSDOS
tc_gtdata(tcf, fp, buf)	/* by Yasha */
char *tcf;		/* tcf: termcap (by Yasha) */
/*tc_gtdata(fp, buf)*/
FILE *fp;
char *buf;
{
	short i;
	boolean first = 1;

	if (tcf != NULL)	/* by Yasha */
		buf = tcf;	/* by Yasha */

	do {
		if (!first) {
			if ((buf[0] != TAB) && (buf[0] != ' ')) {
				break;
			}
		}
		first = 0;
		i = 0;
		while (buf[i]) {
			while (buf[i] && (buf[i] != ':')) {
				i++;
			}
			if (buf[i] == ':') {
				if (!strncmp(buf + i, ":cl=", 4)) {
					tc_gets(buf + i, &CL);
				} else if (!strncmp(buf + i, ":cm=", 4)) {
					tc_gets(buf + i, &CM);
				} else if (!strncmp(buf + i, ":up=", 4)) {
					tc_gets(buf + i, &UC);
				} else if (!strncmp(buf + i, ":do=", 4)) {
					tc_gets(buf + i, &DO);
				} else if (!strncmp(buf + i, ":vs=", 4)) {
					tc_gets(buf + i, &VS);
				} else if (!strncmp(buf + i, ":ve=", 4)) {
					tc_gets(buf + i, &VE);
				} else if (!strncmp(buf + i, ":ti=", 4)) {
					tc_gets(buf + i, &TI);
				} else if (!strncmp(buf + i, ":te=", 4)) {
					tc_gets(buf + i, &TE);
				} else if (!strncmp(buf + i, ":vs=", 4)) {
					tc_gets(buf + i, &VS);
				} else if (!strncmp(buf + i, ":ve=", 4)) {
					tc_gets(buf + i, &VE);
				} else if (!strncmp(buf + i, ":so=", 4)) {
					tc_gets(buf + i, &SO);
				} else if (!strncmp(buf + i, ":se=", 4)) {
					tc_gets(buf + i, &SE);
				} else if (!strncmp(buf + i, ":li#", 4)) {
					tc_gnum(buf + i, &LINES);
				} else if (!strncmp(buf + i, ":co#", 4)) {
					tc_gnum(buf + i, &COLS);
			/* by Yasha */
				} else if (!strncmp(buf + i, ":tc=", 4)) {
					tc_gets(buf + i, &TC);
				}
				i++;
			}
		}
	} while (tcf == NULL && fgets(buf, BUFLEN, fp) != NULL); /* by Yasha */
/*	} while (fgets(buf, BUFLEN, fp) != NULL);*/

/*	if ((!CM) || (!CL)) {
		clean_up("Terminal and termcap must have cm and cl");
	}
	tc_cmget();*/
}
#endif

#ifndef MSDOS
tc_gets(ibuf, tcstr)
char *ibuf;
char **tcstr;
{
	short i, j, k, n;
	char obuf[BUFLEN];

	i = 4;
	j = 0;

	while (ibuf[i] && is_digit(ibuf[i])) {
		i++;
	}

	while (ibuf[i] && (ibuf[i] != ':')) {
		if (ibuf[i] == '\\') {
			i++;
			switch(ibuf[i]) {
			case 'E':
				obuf[j] = ESC;
				i++;
				break;
			case 'n':
				obuf[j] = LF;
				i++;
				break;
			case 'r':
				obuf[j] = CR;
				i++;
				break;
			case 'b':
				obuf[j] = BS;
				i++;
				break;
			case 't':
				obuf[j] = TAB;
				i++;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				n = 0;
				k = 0;
				while (k < 3 && ibuf[i] && is_digit(ibuf[i])) {
					n = (8 * n) + (ibuf[i] - '0');
					i++;
					k++;
				}
				obuf[j] = (char) n;
				break;
			default:
				obuf[j] = ibuf[i];
				i++;
			}
		} else if (ibuf[i] == '^') {
			obuf[j] = ibuf[i+1] - 64;
			i += 2;
		} else {
			obuf[j] = ibuf[i++];
		}
		j++;
	}
	obuf[j] = 0;
	if (!(*tcstr = md_malloc(j + 1))) {
		clean_up("cannot alloc() memory");
	}
	strcpy(*tcstr, obuf);
}
#endif

#ifndef MSDOS
tc_gnum(ibuf, n)
char *ibuf;
int *n;
{
	short i;
	int r = 0;

	i = 4;

	while (is_digit(ibuf[i])) {
		r = (r * 10) + (ibuf[i] - '0');
		i++;
	}
	*n = r;
}
#endif

#ifndef MSDOS
tstp()
{
	endwin();
	md_tstp();

	start_window();
	putstr(TI);
	putstr(VS);
	wrefresh(curscr);
	md_slurp();
}
#endif

#ifndef MSDOS
tc_cmget()
{
	short i = 0, j = 0, rc_spec = 0;

	while (CM[i] && (CM[i] != '%') && (j < 15)) {
		cm_esc[j++] = CM[i++];
	}
	cm_esc[j] = 0;

	while (CM[i] && (rc_spec < 2)) {
		if (CM[i] == '%') {
			i++;
			switch(CM[i]) {
			case 'd':
				rc_spec++;
				break;
			case 'i':
				cm_inc = 1;
				break;
			case '2':
				cm_two = 1;
				rc_spec++;
				break;
			case '3':
				cm_three = 1;
				rc_spec++;
				break;
			case '.':
				cm_char = 1;
				rc_spec++;
				break;
			case 'r':
				cm_reverse = 1;
				break;
			case '+':
				i++;
				cm_inc = CM[i];
				cm_char = 1;
				rc_spec++;
				break;
			}
			i++;
		} else {
			j = 0;
			while (CM[i] && (CM[i] != '%')) {
				cm_sep[j++] = CM[i++];
			}
			cm_sep[j] = 0;
		}
	}

	j = 0;
	if (rc_spec == 2) {
		while (CM[i] && (j < 15)) {
			cm_end[j++] = CM[i++];
		}
	}
	cm_end[j] = 0;
}
#endif

#endif
