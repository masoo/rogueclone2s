/*
 * save.c
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

short write_failed = 0;
char *save_file = "";

extern boolean detect_monster;
extern short cur_level, max_level;
extern char hunger_str[];
extern char login_name[];
extern short party_room;
extern short party_counter;
extern short foods;
extern boolean is_wood[];
extern short cur_room;
extern boolean being_held;
extern short bear_trap;
extern short halluc;
extern short blind;
extern short confused;
extern short levitate;
extern short haste_self;
extern boolean see_invisible;
extern boolean detect_monster;
extern boolean wizard;
extern boolean score_only;
extern short m_moves;

extern boolean msg_cleared;

save_game()
{
	char fname[64];

#ifdef JAPAN
	if (!get_input_line("セーブするファイル名は？", save_file, fname,
			"ゲームのセーブを中止しました。", 0, 1)) {
#else
	if (!get_input_line("File name?", save_file, fname,
			"Game not saved", 0, 1)) {
#endif
		return;
	}
	check_message();
	message(fname, 0);
	save_into_file(fname);
}

save_into_file(sfile)
char *sfile;
{
	FILE *fp;
	int file_id;
	char name_buffer[80];
	char *hptr;
	struct rogue_time rt_buf;

#ifndef ORIGINAL
	if (org_dir && *org_dir)
		md_chdir(org_dir);
#endif
	if (sfile[0] == '~') {
		if (hptr = md_getenv("HOME")) {
#ifdef MSDOS
			hptr = strcpy(name_buffer, hptr);
			while (*hptr)
				hptr++;
			if (hptr[-1] == '\\')
				hptr--;
			strcpy(hptr, sfile+1);
#else
			(void) strcpy(name_buffer, hptr);
			(void) strcat(name_buffer, sfile+1);
#endif
			sfile = name_buffer;
		}
	}
#ifdef UNIX
	if (	((fp = fopen(sfile, "w")) == NULL) ||
			((file_id = md_get_file_id(sfile)) == -1)) {
#ifdef JAPAN
		message("セーブファイルにアクセスできません。", 0);
#else
		message("Problem accessing the save file", 0);
#endif
		goto err_return;
	}
#endif
#ifdef MSDOS
	if (	((fp = fopen(sfile, "wb")) == NULL) ||
			((file_id = md_get_file_id(sfile)) == -1)) {
#ifdef JAPAN
		message("セーブファイルにアクセスできません。", 0);
#else
		message("Problem accessing the save file", 0);
#endif
		goto err_return;
	}
#endif
	md_ignore_signals();
	write_failed = 0;
	(void) xxx(1);
	r_write(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_write(fp, (char *) &cur_level, sizeof(cur_level));
	r_write(fp, (char *) &max_level, sizeof(max_level));
	write_string(hunger_str, fp);
	write_string(login_name, fp);
	r_write(fp, (char *) &party_room, sizeof(party_room));
	r_write(fp, (char *) &party_counter, sizeof(party_counter));
	write_pack(&level_monsters, fp);
	write_pack(&level_objects, fp);
	r_write(fp, (char *) &file_id, sizeof(file_id));
	rw_dungeon(fp, 1);
	r_write(fp, (char *) &foods, sizeof(foods));
	r_write(fp, (char *) &rogue, sizeof(fighter));
	write_pack(&rogue.pack, fp);
	rw_id(id_potions, fp, POTIONS, 1);
	rw_id(id_scrolls, fp, SCROLS, 1);
	rw_id(id_wands, fp, WANDS, 1);
	rw_id(id_rings, fp, RINGS, 1);
	r_write(fp, (char *) traps, (MAX_TRAPS * sizeof(trap)));
	r_write(fp, (char *) is_wood, (WANDS * sizeof(boolean)));
	r_write(fp, (char *) &cur_room, sizeof(cur_room));
	rw_rooms(fp, 1);
	r_write(fp, (char *) &being_held, sizeof(being_held));
	r_write(fp, (char *) &bear_trap, sizeof(bear_trap));
	r_write(fp, (char *) &halluc, sizeof(halluc));
	r_write(fp, (char *) &blind, sizeof(blind));
	r_write(fp, (char *) &confused, sizeof(confused));
	r_write(fp, (char *) &levitate, sizeof(levitate));
	r_write(fp, (char *) &haste_self, sizeof(haste_self));
	r_write(fp, (char *) &see_invisible, sizeof(see_invisible));
	r_write(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_write(fp, (char *) &wizard, sizeof(wizard));
	r_write(fp, (char *) &score_only, sizeof(score_only));
	r_write(fp, (char *) &m_moves, sizeof(m_moves));
	md_gct(&rt_buf);
	rt_buf.second += 10;		/* allow for some processing time */
	r_write(fp, (char *) &rt_buf, sizeof(rt_buf));
	fclose(fp);

	if (write_failed) {
		(void) md_df(sfile);	/* delete file */
	} else {
		clean_up("");
	}

err_return:
	;
#ifndef ORIGINAL
	if (game_dir && *game_dir)
		md_chdir(game_dir);
#endif
}

restore(fname)
char *fname;
{
	FILE *fp;
	struct rogue_time saved_time, mod_time;
	char buf[4];
	char tbuf[40];
	int new_file_id, saved_file_id;

#ifndef ORIGINAL
	if (org_dir && *org_dir)
		md_chdir(org_dir);
#endif

#ifdef UNIX
	if (((new_file_id = md_get_file_id(fname)) == -1) ||
			((fp = fopen(fname, "r")) == NULL)) {
#ifdef JAPAN
		clean_up("ファイルがオープンできませんでした。");
#else
		clean_up("Cannot open file");
#endif
	}
	if (md_link_count(fname) > 1) {
#ifdef JAPAN
		clean_up("ファイルはリンクされています。");
#else
		clean_up("File has link");
#endif
	}
#endif
#ifdef MSDOS
	if (((new_file_id = md_get_file_id(fname)) == -1) ||
			((fp = fopen(fname, "rb")) == NULL)) {
#ifdef JAPAN
		clean_up("ファイルがオープンできませんでした。");
#else
		clean_up("Cannot open file");
#endif
	}
#endif
	(void) xxx(1);
	r_read(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_read(fp, (char *) &cur_level, sizeof(cur_level));
	r_read(fp, (char *) &max_level, sizeof(max_level));
	read_string(hunger_str, fp);

	(void) strcpy(tbuf, login_name);
	read_string(login_name, fp);
	if (strcmp(tbuf, login_name)) {
#ifdef JAPAN
		clean_up("セーブファイルの持ち主が違います。");
#else
		clean_up("You're not the original player");
#endif
	}

	r_read(fp, (char *) &party_room, sizeof(party_room));
	r_read(fp, (char *) &party_counter, sizeof(party_counter));
	read_pack(&level_monsters, fp, 0);
	read_pack(&level_objects, fp, 0);
	r_read(fp, (char *) &saved_file_id, sizeof(saved_file_id));
	if (new_file_id != saved_file_id) {
#ifdef JAPAN
		clean_up("これは元のセーブファイルではありません。");
#else
		clean_up("Sorry, saved game is not in the same file");
#endif
	}
	rw_dungeon(fp, 0);
	r_read(fp, (char *) &foods, sizeof(foods));
	r_read(fp, (char *) &rogue, sizeof(fighter));
	read_pack(&rogue.pack, fp, 1);
	rw_id(id_potions, fp, POTIONS, 0);
	rw_id(id_scrolls, fp, SCROLS, 0);
	rw_id(id_wands, fp, WANDS, 0);
	rw_id(id_rings, fp, RINGS, 0);
	r_read(fp, (char *) traps, (MAX_TRAPS * sizeof(trap)));
	r_read(fp, (char *) is_wood, (WANDS * sizeof(boolean)));
	r_read(fp, (char *) &cur_room, sizeof(cur_room));
	rw_rooms(fp, 0);
	r_read(fp, (char *) &being_held, sizeof(being_held));
	r_read(fp, (char *) &bear_trap, sizeof(bear_trap));
	r_read(fp, (char *) &halluc, sizeof(halluc));
	r_read(fp, (char *) &blind, sizeof(blind));
	r_read(fp, (char *) &confused, sizeof(confused));
	r_read(fp, (char *) &levitate, sizeof(levitate));
	r_read(fp, (char *) &haste_self, sizeof(haste_self));
	r_read(fp, (char *) &see_invisible, sizeof(see_invisible));
	r_read(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_read(fp, (char *) &wizard, sizeof(wizard));
	r_read(fp, (char *) &score_only, sizeof(score_only));
	r_read(fp, (char *) &m_moves, sizeof(m_moves));
	r_read(fp, (char *) &saved_time, sizeof(saved_time));

	if (fread(buf, sizeof(char), 1, fp) > 0) {
		clear();
#ifdef JAPAN
		clean_up("ファイル中によけいな文字があります。");
#else
		clean_up("Extra characters in file");
#endif
	}

	md_gfmt(fname, &mod_time);	/* get file modification time */

	if (has_been_touched(&saved_time, &mod_time)) {
		clear();
#ifdef JAPAN
		clean_up("ファイルが変更されています。");
#else
		clean_up("Sorry, file has been touched");
#endif
	}
	if ((!wizard) && !md_df(fname)) {
#ifdef JAPAN
		clean_up("ファイルを消すことができません。");
#else
		clean_up("Cannot delete file");
#endif
	}
	msg_cleared = 0;
	ring_stats(0);
	fclose(fp);

#ifndef ORIGINAL
	if (game_dir && *game_dir)
		md_chdir(game_dir);
#endif
}

write_pack(pack, fp)
object *pack;
FILE *fp;
{
	object t;

	while (pack = pack->next_object) {
		r_write(fp, (char *) pack, sizeof(object));
	}
	t.ichar = t.what_is = 0;
	r_write(fp, (char *) &t, sizeof(object));
}

read_pack(pack, fp, is_rogue)
object *pack;
FILE *fp;
boolean is_rogue;
{
	object read_obj, *new_obj;

	for (;;) {
		r_read(fp, (char *) &read_obj, sizeof(object));
		if (read_obj.ichar == 0) {
			pack->next_object = (object *) 0;
			break;
		}
		new_obj = alloc_object();
		*new_obj = read_obj;
		if (is_rogue) {
			if (new_obj->in_use_flags & BEING_WORN) {
				do_wear(new_obj);
			} else if (new_obj->in_use_flags & BEING_WIELDED) {
				do_wield(new_obj);
			} else if (new_obj->in_use_flags & (ON_EITHER_HAND)) {
				do_put_on(new_obj,
					((new_obj->in_use_flags & ON_LEFT_HAND)
								? 1 : 0));
			}
		}
		pack->next_object = new_obj;
		pack = new_obj;
	}
}

rw_dungeon(fp, rw)
FILE *fp;
boolean rw;
{
	short i, j;
	char buf[DCOLS];

	for (i = 0; i < DROWS; i++) {
		if (rw) {
			r_write(fp, (char *) dungeon[i], (DCOLS * sizeof(dungeon[0][0])));
#if defined(JAPAN) && !defined(CURSES)
			if (i == DROWS - 1) {				/* by Yasha */
				r_write(fp, get_status_line(), DCOLS);	/* by Yasha */
				break;					/* by Yasha */
			}						/* by Yasha */
#endif
			for (j = 0; j < DCOLS; j++) {
				buf[j] = mvinch(i, j);
			}
			r_write(fp, buf, DCOLS);
		} else {
			r_read(fp, (char *) dungeon[i], (DCOLS * sizeof(dungeon[0][0])));
			r_read(fp, buf, DCOLS);

#if defined(JAPAN) && !defined(CURSES)
			if (i == DROWS - 1) {			/* by Yasha */
				mvaddstr(DROWS - 1, 0, buf);	/* by Yasha */
				break;				/* by Yasha */
			}					/* by Yasha */
#endif
			for (j = 0; j < DCOLS; j++) {
#ifdef COLOR
				if (i < MIN_ROW || i >= DROWS - 1)
					mvaddch(i, j, (unsigned char) buf[j]);
				else
					mvaddch(i, j, colored(buf[j]));
#else
				mvaddch(i, j, (unsigned char) buf[j]);
#endif
			}
		}
	}
}

rw_id(id_table, fp, n, wr)
struct id id_table[];
FILE *fp;
int n;
boolean wr;
{
	short i;

	for (i = 0; i < n; i++) {
		if (wr) {
			r_write(fp, (char *) &(id_table[i].value), sizeof(short));
			r_write(fp, (char *) &(id_table[i].id_status),
				sizeof(unsigned short));
			write_string(id_table[i].title, fp);
		} else {
			r_read(fp, (char *) &(id_table[i].value), sizeof(short));
			r_read(fp, (char *) &(id_table[i].id_status),
				sizeof(unsigned short));
			read_string(id_table[i].title, fp);
		}
	}
}

write_string(s, fp)
char *s;
FILE *fp;
{
	short n;

	n = strlen(s) + 1;
	xxxx(s, n);
	r_write(fp, (char *) &n, sizeof(short));
	r_write(fp, s, n);
}

read_string(s, fp)
char *s;
FILE *fp;
{
	short n;

	r_read(fp, (char *) &n, sizeof(short));
	r_read(fp, s, n);
	xxxx(s, n);
}

rw_rooms(fp, rw)
FILE *fp;
boolean rw;
{
	short i;

	for (i = 0; i < MAXROOMS; i++) {
		rw ? r_write(fp, (char *) (rooms + i), sizeof(room)) :
			r_read(fp, (char *) (rooms + i), sizeof(room));
	}
}

r_read(fp, buf, n)
FILE *fp;
char *buf;
int n;
{
	if (fread(buf, sizeof(char), n, fp) != n) {
#ifdef JAPAN
		clean_up("ファイルが読めません。");
#else
		clean_up("Read() failed, don't know why");
#endif
	}
}

r_write(fp, buf, n)
FILE *fp;
char *buf;
int n;
{
	if (!write_failed) {
		if (fwrite(buf, sizeof(char), n, fp) != n) {
#ifdef JAPAN
			message("ファイルに書けません。", 0);
#else
			message("Write() failed, don't know why", 0);
#endif
			sound_bell();
			write_failed = 1;
		}
	}
}

#ifndef ORIGINAL
boolean
has_been_touched(saved_time, mod_time)
struct rogue_time *saved_time, *mod_time;
{
	register short *sav, *mod;
	int i;

	sav = (short *)saved_time;
	mod = (short *)mod_time;
	for (i = 0; i < 6; i++) { 
		if (*sav < *mod)
			return 1;
		else if (*sav++ > *mod++)
			return 0;
	}
	return 0;
}
#else
boolean
has_been_touched(saved_time, mod_time)
register struct rogue_time *saved_time, *mod_time;
{
	if (saved_time->year < mod_time->year) {
		return(1);
	} else if (saved_time->year > mod_time->year) {
		return(0);
	}
	if (saved_time->month < mod_time->month) {
		return(1);
	} else if (saved_time->month > mod_time->month) {
		return(0);
	}
	if (saved_time->day < mod_time->day) {
		return(1);
	} else if (saved_time->day > mod_time->day) {
		return(0);
	}
	if (saved_time->hour < mod_time->hour) {
		return(1);
	} else if (saved_time->hour > mod_time->hour) {
		return(0);
	}
	if (saved_time->minute < mod_time->minute) {
		return(1);
	} else if (saved_time->minute > mod_time->minute) {
		return(0);
	}
	if (saved_time->second < mod_time->second) {
		return(1);
	}
	return(0);
}
#endif
