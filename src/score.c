/*
 * score.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#include <stdio.h>
#include "rogue.h"

#ifdef UNIX
char *score_file = "rogue.scores";
#endif
#ifdef MSDOS
char *score_file = "rogue.sco";
#endif

extern char login_name[];
extern char *m_names[];
extern short cur_level, max_level;
extern boolean score_only, show_skull, msg_cleared;
extern char *byebye_string, *nick_name;
#ifdef COLOR
extern boolean do_color;
#endif

#ifndef ORIGINAL
#ifdef JAPAN
killed_by(monster, other)
object *monster;
short other;
{
	int i;
	char *p, *q;
	char buf[80];
	char buf2[20];
	struct rogue_time rt;
	static char xpos[] = {
		DCOLS/2-5, DCOLS/2-6, DCOLS/2-7, DCOLS/2-8, DCOLS/2-9,
		DCOLS/2-10, DCOLS/2-10, DCOLS/2-10, DCOLS/2-10, DCOLS/2-10,
		DCOLS/2-10, DCOLS/2-10, DCOLS/2-11, DCOLS/2-19
	};		/* by Yasha */
/*	static char xpos[] = {35,34,33,32,31,30,30,30,30,30,30,30,29,21};*/
	static char *str[] = {
		              "----------",
		             "/          \\",
		            "/            \\",
		           "/              \\",
		          "/                \\",
		         "/                  \\",
		         "|                  |",
		         "|                  |",
		         "|                  |",
		         "|                  |",
		         "|                  |",
		         "|                  |",
		        "*|     *  *  *      | *",
		"________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______"
	};
	static char *os1[] = {
		"", mesg[168], mesg[169], mesg[170], mesg[171]
	};
	static char *os2[] = {
		"", mesg[172], mesg[173], mesg[174], mesg[175]
	};

	md_ignore_signals();

	if (other != QUIT) {
		rogue.gold = ((rogue.gold * 9) / 10);
	}

	if (other) {
		p = os1[other];
		q = os2[other];
	} else {
		p = m_names[monster->m_char - 'A'];
		q = mesg[176];
	}
	(void) strcpy(buf, p);
	(void) strcpy(buf2, q);

	if (show_skull && other != QUIT) {
		md_gct(&rt);
		clear();
#ifdef COLOR
		color(GREEN);
#endif
		for (i = 0; i < 14; i++)
			mvaddstr(i+3, xpos[i], str[i]);
#ifdef COLOR
		color(YELLOW);
		mvaddch(15, DCOLS/2-11, '*');	/* by Yasha */
		mvaddch(15, DCOLS/2-4, '*');	/* by Yasha */
		mvaddch(15, DCOLS/2-1, '*');	/* by Yasha */
		mvaddch(15, DCOLS/2+2, '*');	/* by Yasha */
		mvaddch(15, DCOLS/2+11, '*');	/* by Yasha */
/*		mvaddch(15, 29, '*');
		mvaddch(15, 36, '*');
		mvaddch(15, 39, '*');
		mvaddch(15, 42, '*');
		mvaddch(15, 51, '*');*/
#endif
#ifdef COLOR
		color(YELLOW);
#endif
		center(6, mesg[177]);
		center(7, mesg[178]);
#ifdef COLOR
		standend();
#endif
		center( 9, nick_name);
		center(12, buf);
		center(13, buf2);
		strcpy(buf, mesg[180]);
		lznum(buf+2, rogue.gold, 0);
		center(10, buf);
		*buf = 0;
		znum(buf, rt.year + 1900, 0);
		center(14, buf);
		check_message();
		message("", 0);
	} else {
		(void) strcat(buf, buf2);
		(void) strcat(buf, mesg[181]);
		lznum(buf, rogue.gold, 0);
		strcat(buf, "。");
		message(buf, 0);
	}
	message("", 0);
	put_scores(monster, other);
}
#else /*JAPAN*/
killed_by(monster, other)
object *monster;
short other;
{
	int i;
	char *p, *q;
	char buf[80];
	char buf2[20];
	struct rogue_time rt;
	static char xpos[] = {
		DCOLS/2-5, DCOLS/2-6, DCOLS/2-7, DCOLS/2-8, DCOLS/2-9,
		DCOLS/2-10, DCOLS/2-10, DCOLS/2-10, DCOLS/2-10,
		DCOLS/2-10, DCOLS/2-10, DCOLS/2-11, DCOLS/2-19
	};		/* by Yasha */
/*	static char xpos[] = {35,34,33,32,31,30,30,30,30,30,30,29,21};*/
	static char *str[] = {
		              "----------",
		             "/          \\",
		            "/            \\",
		           "/              \\",
		          "/                \\",
		         "/                  \\",
		         "|                  |",
		         "|                  |",
		         "|                  |",
		         "|                  |",
		         "|                  |",
		        "*|     *  *  *      | *",
		"________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______"
	};
	static char *os1[] = { "", mesg[172], mesg[173], mesg[174], mesg[175] };
	static char *os2[] = { "", mesg[168], mesg[169], mesg[170], mesg[171] };

	md_ignore_signals();

	if (other != QUIT) {
		rogue.gold = ((rogue.gold * 9) / 10);
	}

	if (other) {
		(void) strcpy(buf,  os1[other]);
		(void) strcpy(buf2, os2[other]);
	} else {
		(void) strcpy(buf, mesg[176]);
		if (is_vowel(m_names[monster->m_char - 'A'][0])) {
			(void) strcat(buf, "n");
		}
		(void) strcpy(buf2, m_names[monster->m_char - 'A']);
	}
	if (show_skull && other != QUIT) {
		md_gct(&rt);
		clear();
#ifdef COLOR
		color(GREEN);
#endif
		for (i = 0; i < 13; i++)
			mvaddstr(i+4, xpos[i], str[i]);
#ifdef COLOR
		color(YELLOW);
#endif
		center( 6, mesg[177]);
		center( 7, mesg[178]);
		center( 8, mesg[179]);
#ifdef COLOR
		standend();
#endif
		center(10, nick_name);
		center(12, buf);
		center(13, buf2);
		sprintf(buf, mesg[180], rogue.gold);
		center(11, buf);
		sprintf(buf, "%d", rt.year + 1900);
		center(14, buf);
		check_message();
		message("", 0);
	} else {
		if (*buf2)
			(void) strcat(buf, " ");
		(void) strcat(buf, buf2);
/*		(void) strcat(buf, " with ");*/
		sprintf(buf+strlen(buf), mesg[181], rogue.gold);
		message(buf, 0);
	}
	message("", 0);
	put_scores(monster, other);
}
#endif /*JAPAN*/
#else /*if ORIGINAL*/
killed_by(monster, other)
object *monster;
short other;
{
	char *p;
	char buf[80];

	md_ignore_signals();

	if (other != QUIT) {
		rogue.gold = ((rogue.gold * 9) / 10);
	}

	if (other) {
		switch(other) {
		case HYPOTHERMIA:
			p = "died of hypothermia";
			break;
		case STARVATION:
			p = "died of starvation";
			break;
		case POISON_DART:
			p = "killed by a dart";
			break;
		case QUIT:
			p = "quit";
			break;
		}
		(void) strcpy(buf, p);
	} else {
		(void) strcpy(buf, "killed by ");
		if (is_vowel(m_names[monster->m_char - 'A'][0])) {
			(void) strcat(buf, "an ");
		} else {
			(void) strcat(buf, "a ");
		}
		(void) strcat(buf, m_names[monster->m_char - 'A']);
	}
	(void) strcat(buf, " with ");
	sprintf(buf+strlen(buf), "%ld gold", rogue.gold);
	if ((!other) && show_skull) {
		clear();
		mvaddstr( 4, 32,      "__---------__");
		mvaddstr( 5, 30,    "_~             ~_");
		mvaddstr( 6, 29,   "/                 \\");
		mvaddstr( 7, 28,  "~                   ~");
		mvaddstr( 8, 27, "/                     \\");
		mvaddstr( 9, 27, "|    XXXX     XXXX    |");
		mvaddstr(10, 27, "|    XXXX     XXXX    |");
		mvaddstr(11, 27, "|    XXX       XXX    |");
		mvaddstr(12, 28, "\\         @         /");
		mvaddstr(13, 29,  "--\\     @@@     /--");
		mvaddstr(14, 30,    "| |    @@@    | |");
		mvaddstr(15, 30,    "| |           | |");
		mvaddstr(16, 30,    "| vvVvvvvvvvVvv |");
		mvaddstr(17, 30,    "|  ^^^^^^^^^^^  |");
		mvaddstr(18, 31,    "\\_           _/");
		mvaddstr(19, 33,       "~---------~");
		center(21, nick_name);
		center(22, buf);
	} else {
		message(buf, 0);
	}
	message("", 0);
	put_scores(monster, other);
}
#endif /*if ORIGINAL*/

win()
{
#ifndef ORIGINAL
	register int i;
#define ________  0x00
#define _______O  0x01
#define ____O___  0x08
#define ____O_O_  0x0a
#define ____OO_O  0x0d
#define __O_____  0x20
#define __O_O___  0x28
#define __O_O__O  0x29
#define _OOO___O  0x71
#define _OOOO__O  0x79
#define _OOOO_O_  0x7a
#define O_______  0x80
#define O_____O_  0x82
#define O___O___  0x88
#define O___O__O  0x89
#define O___O_O_  0x8a
#define O___O_OO  0x8b
#define O__OOO__  0x9c
#define O__OOOO_  0x9e
#define O_O___O_  0xa2
#define OO______  0xc0
#define OO_____O  0xc1
#define OO___OO_  0xc6
#define OO__O___  0xc8
#define OOO_____  0xe0
	static char ban[7][8] = {
    {O___O___,________,____O___,O_______,____O___,_______O,OO__O___,__O_____},
    {O___O___,________,____OO_O,O_______,____O___,________,O___O___,__O_____},
    {O___O__O,OO__O___,O___O_O_,O__OOO__,_OOOO__O,OO______,O__OOO__,__O_____},
    {_OOOO_O_,__O_O___,O___O___,O_____O_,O___O_O_,__O_____,O___O___,__O_____},
    {____O_O_,__O_O___,O___O___,O__OOOO_,O___O_OO,OOO_____,O___O___,__O_____},
    {O___O_O_,__O_O__O,O___O___,O_O___O_,O___O_O_,________,O___O__O,________},
    {_OOO___O,OO___OO_,O___O___,O__OOOO_,_OOOO__O,OO_____O,OO___OO_,__O_____}
	};
#endif

	unwield(rogue.weapon);		/* disarm and relax */
	unwear(rogue.armor);
	un_put_on(rogue.left_ring);
	un_put_on(rogue.right_ring);

	clear();
#ifndef ORIGINAL
	for (i = 0; i < 7; i++)
		mvaddbanner(i+6, DCOLS/2-30, ban[i]);
/*		mvaddbanner(i+6, 10, ban[i]);*/
#ifdef COLOR
	color(YELLOW);
#endif
	center(15, mesg[182]);
	center(16, mesg[183]);
	center(17, mesg[184]);
	center(18, mesg[185]);
#ifdef COLOR
	standend();
#endif
#else
	mvaddstr(10, 11, "@   @  @@@   @   @      @  @  @   @@@   @   @   @");
	mvaddstr(11, 11, " @ @  @   @  @   @      @  @  @  @   @  @@  @   @");
	mvaddstr(12, 11, "  @   @   @  @   @      @  @  @  @   @  @ @ @   @");
	mvaddstr(13, 11, "  @   @   @  @   @      @  @  @  @   @  @  @@");
	mvaddstr(14, 11, "  @    @@@    @@@        @@ @@    @@@   @   @   @");
	mvaddstr(17, 11, "Congratulations,  you have  been admitted  to  the");
	mvaddstr(18, 11, "Fighters' Guild.   You return home,  sell all your");
	mvaddstr(19, 11, "treasures at great profit and retire into comfort.");
#endif
	message("", 0);
	message("", 0);
	id_all();
	sell_pack();
	put_scores((object *) 0, WIN);
}

#ifndef ORIGINAL
mvaddbanner(row, col, ban)
short row, col;
register char *ban;
{
	register int i;
#ifdef COLOR
#ifdef HUMAN
	int rev = ' ' | (12 << 8);	/* by Yasha (reverse cyan) */
#else
	int rev = ' ' | (RGREEN << 8);
#endif
#endif

	move(row, col);
	for (i = 0; i < 59; i++) {
		if (ban[i >> 3] & (0x80 >> (i & 7)))
#ifdef COLOR
			addch(rev);
#else
			addch('@');
#endif
		else
			addch(' ');
	}
}
#endif

quit(from_intrpt)
boolean from_intrpt;
{
	char buf[128];
	short i, orow, ocol;
	boolean mc;

	md_ignore_signals();

	if (from_intrpt) {

		orow = rogue.row;
		ocol = rogue.col;
		mc = msg_cleared;

		for (i = 0; i < DCOLS; i++) {
			buf[i] = mvinch(0, i);
		}
	}
	check_message();
#ifdef JAPAN
	message("ゲームを終了してよいのですか？", 1);
#else
	message("Really quit?", 1);
#endif
	if (rgetchar() != 'y') {
		md_heed_signals();
		check_message();
		if (from_intrpt) {
			for (i = 0; i < DCOLS; i++) {
				mvaddch(0, i, (unsigned char) buf[i]);
			}
			msg_cleared = mc;
			move(orow, ocol);
			refresh();
		}
		return;
	}
	if (from_intrpt) {
		clean_up(mesg[12]);	/* byebye_string */
	}
	check_message();
	killed_by((object *) 0, QUIT);
}

#ifndef ORIGINAL
put_scores(monster, other)
object *monster;
short other;
{
	short i, n, ne, rank;
#ifndef TOPSCO
	short found_pos;
#endif
	char scores[10][82], n_names[10][30];
	char *p, buf[100], file[100];
	FILE *fp;

#ifdef UNIX
	fp = NULL;
	if ((!game_dir || !*game_dir) && (p = md_getenv("HOME")) != NULL) {
		p = strcpy(file, p);
		while (*p)
			p++;
		if (p[-1] != '/')
			*p++ = '/';
		strcpy(p, score_file);
		if ((fp = fopen(file, "r+")) == NULL)
			fp = fopen(file, "w+");
	}
	if (fp == NULL) {
		strcpy(file, score_file);
		if ((fp = fopen(file, "r+")) == NULL)
			fp = fopen(file, "w+");
	}
	if (fp == NULL) {
		message(mesg[186], 0);
		sf_error();
	}
#endif
#ifdef MSDOS
	fp = NULL;
	if ((!game_dir || !*game_dir) && (p = md_getenv("HOME")) != NULL) {
		p = strcpy(file, p);
		while (*p)
			p++;
		if (p[-1] != '\\')
			*p++ = '\\';
		strcpy(p, score_file);
		if ((fp = fopen(file, "rb+")) == NULL)
			fp = fopen(file, "wb+");
	}
	if (fp == NULL) {
		strcpy(file, score_file);
		if ((fp = fopen(file, "rb+")) == NULL)
			fp = fopen(file, "wb+");
	}
	if (fp == NULL) {
		message(mesg[186], 0);
		sf_error();
	}
#endif
	(void) xxx(1);
	for (i = 0; i < 10; i++) {
		if ((n = fread(scores[i], sizeof(char), 80, fp)) == 0)
			break;
		if (n < 80)
			sf_error();
		xxxx(scores[i], 80);
		if ((n = fread(n_names[i],sizeof(char),30,fp)) < 30)
			sf_error();
		xxxx(n_names[i], 30);
	}
	fclose(fp);
	ne = i;

#ifndef TOPSCO
	found_pos = -1;
	for (i = 0; i < ne && !score_only; i++) {
		if (name_cmp(scores[i]+15, login_name))
			continue;
		for (p = scores[i] + 5; *p == ' '; p++)
			/* void */;
		if (rogue.gold > lget_number(p))
			found_pos = i;
		else
			score_only = 1;
	}
	if (found_pos != -1) {
		ne--;
		for (i = found_pos; i < ne; i++) {
			(void) strcpy(scores[i], scores[i+1]);
			(void) strcpy(n_names[i], n_names[i+1]);
		}
	}
#endif
	rank = 10;
	if (!score_only) {
		for (i = 0; i < ne; i++) {
			for (p = scores[i] + 5; *p == ' '; p++)
				/* void */;
			if (rogue.gold <= lget_number(p))
				continue;
			rank = i;
			break;
		}
		if (ne == 0)
			rank = 0;
		else if (ne < 10 && rank == 10)
			rank = ne;
		if (rank < 10) {
			insert_score(scores, n_names, nick_name,
					rank, ne, monster, other);
			if (ne < 10)
				ne++;
		}
	}

	md_ignore_signals();
	clear();
#ifdef COLOR
	color(YELLOW);
#endif
#ifdef JAPAN
	mvaddstr(3, 20, mesg[187]);
#else
	mvaddstr(3, 25, mesg[187]);
#endif
#ifdef COLOR
	color(GREEN);
#endif
	mvaddstr(6, 0, mesg[188]);
#ifdef COLOR
	standend();
#endif
	for (i = 0; i < ne; i++) {
		scores[i][1] = (i == 9)? '1': ' ';
		scores[i][2] = (i == 9)? '0': '1' + i;
		nickize(buf, scores[i], n_names[i]);
		if (i == rank) {
#ifdef COLOR
			if (do_color)
				color(RCYAN);
			else
				standout();
#else
			standout();
#endif
			mvaddstr(i+8, 0, buf);
			standend();
		} else
			mvaddstr(i+8, 0, buf);
	}
	refresh();
	if (rank < 10) {
#ifdef UNIX
		if ((fp = fopen(file, "w")) == NULL) {
#endif
#ifdef MSDOS
		if ((fp = fopen(file, "wb")) == NULL) {
#endif
			message(mesg[186], 0);
			sf_error();
		}
		(void) xxx(1);
		for (i = 0; i < ne; i++) {
			xxxx(scores[i], 80);
			fwrite(scores[i], sizeof(char), 80, fp);
			xxxx(n_names[i], 30);
			fwrite(n_names[i], sizeof(char), 30, fp);
		}
		fclose(fp);
	}
	message("", 0);
	clean_up("");
}
#else /* if ORIGINAL */
put_scores(monster, other)
object *monster;
short other;
{
	short i, n, rank = 10, x, ne = 0, found_player = -1;
	char scores[10][82];
	char n_names[10][30];
	char buf[100];
	FILE *fp;
	long s;
	boolean failed = 0;
#ifdef UNIX
	char *mode = "r+w";
#endif
#ifdef MSDOS
	char *mode = "rb+";
#endif

	while ((fp = fopen(score_file, mode)) == NULL) {
		if (!failed) {
#ifdef UNIX
			mode = "w";
#endif
#ifdef MSDOS
			mode = "wb+";
#endif
		} else {
			message("Cannot read/write/create score file", 0);
			sf_error();
		}
		failed = 1;
	}
	(void) xxx(1);

	for (i = 0; i < 10; i++) {
		if (((n=fread(scores[i],sizeof(char),80,fp))<80) && (n!=0)) {
			sf_error();
		} else if (n != 0) {
			xxxx(scores[i], 80);
			if ((n = fread(n_names[i],sizeof(char),30,fp)) < 30) {
				sf_error();
			}
			xxxx(n_names[i], 30);
		} else {
			break;
		}
		ne++;
		if (!score_only) {
			if (!name_cmp(scores[i]+15, login_name)) {
				x = 5;
				while (scores[i][x] == ' ') {
					x++;
				}
				s = lget_number(scores[i] + x);
				if (rogue.gold < s) {
					score_only = 1;
				} else {
					found_player = i;
				}
			}
		}
	}
	if (found_player != -1) {
		ne--;
		for (i = found_player; i < ne; i++) {
			(void) strcpy(scores[i], scores[i+1]);
			(void) strcpy(n_names[i], n_names[i+1]);
		}
	}
	if (!score_only) {
		for (i = 0; i < ne; i++) {
			x = 5;
			while (scores[i][x] == ' ') {
				x++;
			}
			s = lget_number(scores[i] + x);

			if (rogue.gold >= s) {
				rank = i;
				break;
			}
		}
		if (ne == 0) {
			rank = 0;
		} else if ((ne < 10) && (rank == 10)) {
			rank = ne;
		}
		if (rank < 10) {
			insert_score(scores, n_names, nick_name, rank,
				ne, monster, other);
			if (ne < 10) {
				ne++;
			}
		}
		rewind(fp);
	}

	clear();
	mvaddstr(3, 30, "Top  Ten  Rogueists");
	mvaddstr(8, 0, "Rank   Score   Name");

	md_ignore_signals();

	(void) xxx(1);

	for (i = 0; i < ne; i++) {
		if (i == rank) {
			standout();
		}
		if (i == 9) {
			scores[i][0] = '1';
			scores[i][1] = '0';
		} else {
			scores[i][0] = ' ';
			scores[i][1] = i + '1';
		}
		nickize(buf, scores[i], n_names[i]);
		mvaddstr(i+10, 0, buf);
		if (rank < 10) {
			xxxx(scores[i], 80);
			fwrite(scores[i], sizeof(char), 80, fp);
			xxxx(n_names[i], 30);
			fwrite(n_names[i], sizeof(char), 30, fp);
		}
		if (i == rank) {
			standend();
		}
	}
	refresh();
	fclose(fp);
	message("", 0);
	clean_up("");
}
#endif /* if ORIGINAL */

#ifdef JAPAN
insert_score(scores, n_names, n_name, rank, n, monster, other)
char scores[][82];
char n_names[][30];
char *n_name;
short rank, n;
object *monster;
{
	short i;
	char *p;
	char buf[82];

	if (n > 0) {
		for (i = n; i > rank; i--) {
			if ((i < 10) && (i > 0)) {
				(void) strcpy(scores[i], scores[i-1]);
				(void) strcpy(n_names[i], n_names[i-1]);
			}
		}
	}
	sprintf(buf, " %2d   %6ld   %s: ", rank+1, rogue.gold, login_name);

	if (other != WIN) {
		if (has_amulet())
			(void) strcat(buf, mesg[189]);
		strcat(buf, mesg[190]);
#ifndef ORIGINAL
		znum(buf, cur_level, 0);
#else
		znum(buf, max_level, 0);
#endif
		strcat(buf, mesg[191]);
	}
	if (other) {
		switch(other) {
		case HYPOTHERMIA:
			p = mesg[192];
			break;
		case STARVATION:
			p = mesg[193];
			break;
		case POISON_DART:
			p = mesg[194];
			break;
		case QUIT:
			p = mesg[195];
			break;
		case WIN:
			p = mesg[196];
			break;
		}
		(void) strcat(buf, p);
	} else {
		(void) strcat(buf, m_names[monster->m_char - 'A']);
		(void) strcat(buf, mesg[197]);
	}
	strcat(buf, "。");
	for (i = strlen(buf); i < 79; i++) {
		buf[i] = ' ';
	}
	buf[79] = 0;
	(void) strcpy(scores[rank], buf);
	(void) strcpy(n_names[rank], n_name);
}

#else /*JAPAN*/

insert_score(scores, n_names, n_name, rank, n, monster, other)
char scores[][82];
char n_names[][30];
char *n_name;
short rank, n;
object *monster;
{
	short i;
	char *p;
	char buf[82];

	if (n > 0) {
		for (i = n; i > rank; i--) {
			if ((i < 10) && (i > 0)) {
				(void) strcpy(scores[i], scores[i-1]);
				(void) strcpy(n_names[i], n_names[i-1]);
			}
		}
	}
	sprintf(buf, " %2d   %6ld   %s: ", rank+1, rogue.gold, login_name);

	if (other) {
		switch(other) {
		case HYPOTHERMIA:
			p = mesg[192];
			break;
		case STARVATION:
			p = mesg[193];
			break;
		case POISON_DART:
			p = mesg[194];
			break;
		case QUIT:
			p = mesg[195];
			break;
		case WIN:
			p = mesg[196];
			break;
		}
		(void) strcat(buf, p);
	} else {
		(void) strcat(buf, mesg[197]);
		if (is_vowel(m_names[monster->m_char - 'A'][0])) {
			p = "an ";
		} else {
			p = "a ";
		}
		(void) strcat(buf, p);
		(void) strcat(buf, m_names[monster->m_char - 'A']);
	}
#ifndef ORIGINAL
	sprintf(buf+strlen(buf), " on level %d ", cur_level);
#else
	sprintf(buf+strlen(buf), " on level %d ", max_level);
#endif
	if ((other != WIN) && has_amulet()) {
		(void) strcat(buf, mesg[189]);
	}
	for (i = strlen(buf); i < 79; i++) {
		buf[i] = ' ';
	}
	buf[79] = 0;
	(void) strcpy(scores[rank], buf);
	(void) strcpy(n_names[rank], n_name);
}
#endif /*JAPAN*/

is_vowel(ch)
short ch;
{
	return( (ch == 'a') ||
		(ch == 'e') ||
		(ch == 'i') ||
		(ch == 'o') ||
		(ch == 'u') );
}

sell_pack()
{
	object *obj;
	short row = 2, val;
	char buf[80];

	obj = rogue.pack.next_object;

	clear();
	mvaddstr(1, 0, mesg[198]);

	while (obj) {
		if (obj->what_is != FOOD) {
			obj->identified = 1;
			val = get_value(obj);
			rogue.gold += val;

			if (row < DROWS) {
				sprintf(buf, "%5d      ", val);
				get_desc(obj, buf+11, 1);
				mvaddstr(row++, 0, buf);
			}
		}
		obj = obj->next_object;
	}
	refresh();
	if (rogue.gold > MAX_GOLD) {
		rogue.gold = MAX_GOLD;
	}
	message("", 0);
}

get_value(obj)
object *obj;
{
	short wc;
	int val;

	wc = obj->which_kind;

	switch(obj->what_is) {
	case WEAPON:
		val = id_weapons[wc].value;
		if ((wc == ARROW) || (wc == DAGGER) || (wc == SHURIKEN) ||
			(wc == DART)) {
			val *= obj->quantity;
		}
		val += (obj->d_enchant * 85) + (obj->hit_enchant * 85);
		break;
	case ARMOR:
		val = id_armors[wc].value + (obj->d_enchant * 75);
		if (obj->is_protected) {
			val += 200;
		}
		break;
	case WAND:
		val = id_wands[wc].value * (obj->class + 1);
		break;
	case SCROL:
		val = id_scrolls[wc].value * obj->quantity;
		break;
	case POTION:
		val = id_potions[wc].value * obj->quantity;
		break;
	case AMULET:
		val = 5000;
		break;
	case RING:
		val = id_rings[wc].value * (obj->class + 1);
		break;
	}
	if (val <= 0) {
		val = 10;
	}
	return(val);
}

id_all()
{
	short i;

	for (i = 0; i < SCROLS; i++) {
		id_scrolls[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < WEAPONS; i++) {
		id_weapons[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < ARMORS; i++) {
		id_armors[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < WANDS; i++) {
		id_wands[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < POTIONS; i++) {
		id_potions[i].id_status = IDENTIFIED;
	}
}

#ifndef TOPSCO
name_cmp(s1, s2)
char *s1, *s2;
{
	short i = 0;
	int r;

	while(s1[i] != ':') {
#ifndef ORIGINAL
		r = (unsigned char) s1[i];
#ifdef EUC
		if (r & 0x80)		/* by Yasha */
#else	/* Shift JIS */
		if (r > 0x80 && r < 0xa0 || r >= 0xe0 && r < 0xf0)
#endif
			i++;
#endif
		i++;
	}
	s1[i] = 0;
	r = strcmp(s1, s2);
	s1[i] = ':';
	return(r);
}
#else
#ifdef ORIGINAL
name_cmp(s1, s2)
char *s1, *s2;
{
	short i = 0;
	int r;

	while(s1[i] != ':') {
		i++;
	}
	s1[i] = 0;
	r = strcmp(s1, s2);
	s1[i] = ':';
	return(r);
}
#endif /*ORIGINAL*/
#endif /*TOPSCO*/

xxxx(buf, n)
char *buf;
short n;
{
	short i;
#ifndef ORIGINAL
	char c;		/* char is already defined to be unsigned */
#else
	unsigned char c;
#endif

	for (i = 0; i < n; i++) {

		/* It does not matter if accuracy is lost during this assignment */
#ifndef ORIGINAL
		c = (char) xxx(0);
#else
		c = (unsigned char) xxx(0);
#endif

		buf[i] ^= c;
	}
}

long
xxx(st)
boolean st;
{
	static long f, s;
	long r;

	if (st) {
		f = 37;
		s = 7;
		return(0L);
	}
	r = ((f * s) + 9337) % 8887;
	f = s;
	s = r;
	return(r);
}

nickize(buf, score, n_name)
char *buf, *score, *n_name;
{
	short i = 15, j;

	if (!n_name[0]) {
		(void) strcpy(buf, score);
		return;
	}
	(void) strncpy(buf, score, 16);

	while (score[i] != ':') {
		i++;
	}

	(void) strcpy(buf+15, n_name);
	j = strlen(buf);

	while (score[i]) {
		buf[j++] = score[i++];
	}
	buf[j] = 0;
	buf[79] = 0;
}

center(row, buf)
short row;
char *buf;
{
	short margin;

	margin = ((DCOLS - strlen(buf)) / 2);
	mvaddstr(row, margin, buf);
}

sf_error()
{
	message("", 1);
	clean_up(mesg[199]);
}
