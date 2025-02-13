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

#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utf8.h"

#include "display.h"
#include "init.h"
#include "machdep.h"
#include "message.h"
#include "object.h"
#include "pack.h"
#include "ring.h"
#include "rogue.h"
#include "save.h"
#include "score.h"

short write_failed = 0;
char *save_file = "";

extern short cur_level, max_level;
extern char hunger_str[];
extern char login_name[];
extern short party_room;
extern short party_counter;
extern short foods;
extern bool is_wood[];
extern short cur_room;
extern bool being_held;
extern short bear_trap;
extern short halluc;
extern short blind;
extern short confused;
extern short levitate;
extern short haste_self;
extern bool see_invisible;
extern bool detect_monster;
extern bool wizard;
extern bool score_only;
extern short m_moves;

extern bool msg_cleared;

void
save_game(void)
{
	char fname[64];

	if (!get_input_line("セーブするファイル名は？", save_file, fname,
		"ゲームのセーブを中止しました。", 0, 1)) {
		return;
	}
	check_message();
	message(fname, 0);
	save_into_file(fname);
}

void
save_into_file(char *sfile)
{
	FILE *fp;
	int file_id;
	char name_buffer[80];
	char *hptr;
	rogue_time rt_buf;

	if (org_dir && *org_dir) {
		chdir(org_dir);
	}
	if (sfile[0] == '~') {
		if ((hptr = md_ghome())) {
			(void)strcpy(name_buffer, hptr);
			(void)strcat(name_buffer, sfile + 1);
			sfile = name_buffer;
		}
	}
	if (((fp = fopen(sfile, "wb")) == NULL) ||
	    ((file_id = md_get_file_id(sfile)) == -1)) {
		message("セーブファイルにアクセスできません。", 0);
		goto err_return;
	}
	md_ignore_signals();
	write_failed = 0;
	(void)xxx(1);
	r_write(fp, (char *)&detect_monster, sizeof(detect_monster));
	r_write(fp, (char *)&cur_level, sizeof(cur_level));
	r_write(fp, (char *)&max_level, sizeof(max_level));
	write_string(hunger_str, fp);
	write_string(login_name, fp);
	r_write(fp, (char *)&party_room, sizeof(party_room));
	r_write(fp, (char *)&party_counter, sizeof(party_counter));
	write_pack(&level_monsters, fp);
	write_pack(&level_objects, fp);
	r_write(fp, (char *)&file_id, sizeof(file_id));
	rw_dungeon(fp, 1);
	r_write(fp, (char *)&foods, sizeof(foods));
	r_write(fp, (char *)&rogue, sizeof(fighter));
	write_pack(&rogue.pack, fp);
	rw_id(id_potions, fp, POTIONS, 1);
	rw_id(id_scrolls, fp, SCROLS, 1);
	rw_id(id_wands, fp, WANDS, 1);
	rw_id(id_rings, fp, RINGS, 1);
	r_write(fp, (char *)traps, (MAX_TRAPS * sizeof(trap)));
	r_write(fp, (char *)is_wood, (WANDS * sizeof(bool)));
	r_write(fp, (char *)&cur_room, sizeof(cur_room));
	rw_rooms(fp, 1);
	r_write(fp, (char *)&being_held, sizeof(being_held));
	r_write(fp, (char *)&bear_trap, sizeof(bear_trap));
	r_write(fp, (char *)&halluc, sizeof(halluc));
	r_write(fp, (char *)&blind, sizeof(blind));
	r_write(fp, (char *)&confused, sizeof(confused));
	r_write(fp, (char *)&levitate, sizeof(levitate));
	r_write(fp, (char *)&haste_self, sizeof(haste_self));
	r_write(fp, (char *)&see_invisible, sizeof(see_invisible));
	r_write(fp, (char *)&detect_monster, sizeof(detect_monster));
	r_write(fp, (char *)&wizard, sizeof(wizard));
	r_write(fp, (char *)&score_only, sizeof(score_only));
	r_write(fp, (char *)&m_moves, sizeof(m_moves));
	md_gct(&rt_buf);
	rt_buf.second += 10; /* allow for some processing time */
	r_write(fp, (char *)&rt_buf, sizeof(rt_buf));
	fclose(fp);

	if (write_failed) {
		(void)md_df(sfile); /* delete file */
	} else {
		clean_up("");
	}

err_return:;
	if (game_dir && *game_dir) {
		chdir(game_dir);
	}
}

void
restore(char *fname)
{
	FILE *fp = {
		NULL}; /* 未初期化変数の使用の警告除去のため NULL で初期化 */
	rogue_time saved_time, mod_time;
	char buf[4];
	char tbuf[40];
	int new_file_id, saved_file_id;

	if (org_dir && *org_dir) {
		chdir(org_dir);
	}

	if (((new_file_id = md_get_file_id(fname)) == -1) ||
	    ((fp = fopen(fname, "rb")) == NULL)) {
		clean_up("ファイルがオープンできませんでした。");
	}
	if (md_link_count(fname) > 1) {
		clean_up("ファイルはリンクされています。");
	}
	(void)xxx(1);
	r_read(fp, (char *)&detect_monster, sizeof(detect_monster));
	r_read(fp, (char *)&cur_level, sizeof(cur_level));
	r_read(fp, (char *)&max_level, sizeof(max_level));
	read_string(hunger_str, fp);

	(void)strcpy(tbuf, login_name);
	read_string(login_name, fp);
	if (strcmp(tbuf, login_name)) {
		clean_up("セーブファイルの持ち主が違います。");
	}

	r_read(fp, (char *)&party_room, sizeof(party_room));
	r_read(fp, (char *)&party_counter, sizeof(party_counter));
	read_pack(&level_monsters, fp, 0);
	read_pack(&level_objects, fp, 0);
	r_read(fp, (char *)&saved_file_id, sizeof(saved_file_id));
	if (new_file_id != saved_file_id) {
		clean_up("これは元のセーブファイルではありません。");
	}
	rw_dungeon(fp, 0);
	r_read(fp, (char *)&foods, sizeof(foods));
	r_read(fp, (char *)&rogue, sizeof(fighter));
	read_pack(&rogue.pack, fp, 1);
	rw_id(id_potions, fp, POTIONS, 0);
	rw_id(id_scrolls, fp, SCROLS, 0);
	rw_id(id_wands, fp, WANDS, 0);
	rw_id(id_rings, fp, RINGS, 0);
	r_read(fp, (char *)traps, (MAX_TRAPS * sizeof(trap)));
	r_read(fp, (char *)is_wood, (WANDS * sizeof(bool)));
	r_read(fp, (char *)&cur_room, sizeof(cur_room));
	rw_rooms(fp, 0);
	r_read(fp, (char *)&being_held, sizeof(being_held));
	r_read(fp, (char *)&bear_trap, sizeof(bear_trap));
	r_read(fp, (char *)&halluc, sizeof(halluc));
	r_read(fp, (char *)&blind, sizeof(blind));
	r_read(fp, (char *)&confused, sizeof(confused));
	r_read(fp, (char *)&levitate, sizeof(levitate));
	r_read(fp, (char *)&haste_self, sizeof(haste_self));
	r_read(fp, (char *)&see_invisible, sizeof(see_invisible));
	r_read(fp, (char *)&detect_monster, sizeof(detect_monster));
	r_read(fp, (char *)&wizard, sizeof(wizard));
	r_read(fp, (char *)&score_only, sizeof(score_only));
	r_read(fp, (char *)&m_moves, sizeof(m_moves));
	r_read(fp, (char *)&saved_time, sizeof(saved_time));

	if (fread(buf, sizeof(char), 1, fp) > 0) {
		clear();
		clean_up("ファイル中によけいな文字があります。");
	}

	md_gfmt(fname, &mod_time); /* get file modification time */

	if (has_been_touched(&saved_time, &mod_time)) {
		clear();
		clean_up("ファイルが変更されています。");
	}
	fclose(fp);
	if ((!wizard) && !md_df(fname)) {
		clean_up("ファイルを消すことができません。");
	}
	msg_cleared = false;
	ring_stats(0);

	if (game_dir && *game_dir) {
		chdir(game_dir);
	}
}

/*
 * write_pack
 * 全objectのパックの情報をファイルに書き込む
 */
void
write_pack(object *pack, FILE *fp)
{
	while ((pack = pack->next_object)) {
		write_obj(pack, fp);
	}
	object t;
	t.ichar = t.what_is = 0;
	write_obj(&t, fp);
}

/*
 * read_pack
 * 全objectのパックの情報をファイルから読み込む
 */
void
read_pack(object *pack, FILE *fp, bool is_rogue)
{
	object r_obj, *new_obj;

	for (;;) {
		read_obj(&r_obj, fp);
		if (r_obj.ichar == 0) {
			pack->next_object = (object *)0;
			break;
		}
		new_obj = alloc_object();
		*new_obj = r_obj;
		if (is_rogue) {
			if (new_obj->in_use_flags & BEING_WORN) {
				do_wear(new_obj);
			} else if (new_obj->in_use_flags & BEING_WIELDED) {
				do_wield(new_obj);
			} else if (new_obj->in_use_flags & (ON_EITHER_HAND)) {
				do_put_on(new_obj,
				    ((new_obj->in_use_flags & ON_LEFT_HAND)
					    ? 1
					    : 0));
			}
		}
		pack->next_object = new_obj;
		pack = new_obj;
	}
}

/*
 * rw_dungeon
 * セーブデータからのダンジョン情報の保存と復元
 */
void
rw_dungeon(FILE *fp, bool rw)
{
	short i, j;
	char buf[ROGUE_COLUMNS];

	if (rw) { // 書き込み時実行
		for (i = 0; i < ROGUE_LINES; i++) {
			r_write(fp, (char *)dungeon[i],
			    (ROGUE_COLUMNS * sizeof(dungeon[0][0])));
			for (j = 0; j < ROGUE_COLUMNS; j++) {
				buf[j] = mvinch_rogue(i, j);
			}
			r_write(fp, buf, ROGUE_COLUMNS);
		}
		return;
	} else { // 読み込み時実行
		for (i = 0; i < ROGUE_LINES; i++) {
			r_read(fp, (char *)dungeon[i],
			    (ROGUE_COLUMNS * sizeof(dungeon[0][0])));
			r_read(fp, buf, ROGUE_COLUMNS);

			if (i < ROGUE_LINES - 1) {
				for (j = 0; j < ROGUE_COLUMNS; j++) {
					mvaddch_rogue(i, j,
					    (unsigned char)buf[j]);
				}
			} else {
				print_stats(STAT_ALL);
			}
		}
		return;
	}
}

void
rw_id(id id_table[], FILE *fp, int n, bool wr)
{
	short i;

	for (i = 0; i < n; i++) {
		if (wr) {
			r_write(fp, (char *)&(id_table[i].value),
			    sizeof(short));
			r_write(fp, (char *)&(id_table[i].id_status),
			    sizeof(unsigned short));
			write_string(id_table[i].title, fp);
		} else {
			r_read(fp, (char *)&(id_table[i].value), sizeof(short));
			r_read(fp, (char *)&(id_table[i].id_status),
			    sizeof(unsigned short));
			read_string(id_table[i].title, fp);
		}
	}
}

void
write_string(char *s, FILE *fp)
{
	short n;

	n = strlen(s) + 1;
	xxxx(s, n);
	r_write(fp, (char *)&n, sizeof(short));
	r_write(fp, s, n);
}

void
read_string(char *s, FILE *fp)
{
	short n;

	r_read(fp, (char *)&n, sizeof(short));
	r_read(fp, s, n);
	xxxx(s, n);
}

void
rw_rooms(FILE *fp, bool rw)
{
	short i;

	for (i = 0; i < MAXROOMS; i++) {
		rw ? r_write(fp, (char *)(rooms + i), sizeof(room))
		   : r_read(fp, (char *)(rooms + i), sizeof(room));
	}
}

void
r_read(FILE *fp, char *buf, int n)
{
	if (fread(buf, sizeof(char), n, fp) != n) {
		clean_up("ファイルが読めません。");
	}
}

void
r_write(FILE *fp, char *buf, int n)
{
	if (!write_failed) {
		if (fwrite(buf, sizeof(char), n, fp) != n) {
			message("ファイルに書けません。", 0);
			sound_bell();
			write_failed = 1;
		}
	}
}

bool
has_been_touched(rogue_time *saved_time, rogue_time *mod_time)
{
	short *sav, *mod;
	int i;

	sav = (short *)saved_time;
	mod = (short *)mod_time;
	for (i = 0; i < 6; i++) {
		if (*sav < *mod) {
			return true;
		} else if (*sav++ > *mod++) {
			return false;
		}
	}
	return false;
}

/*
 * write_obj
 * オブジェクトの情報をファイルに書き込む
 */
void
write_obj(object *obj, FILE *fp)
{
	fwrite(&obj->m_flags, sizeof(obj->m_flags), 1, fp);

	int32_t damage_size = utf8size(obj->damage);
	fwrite(&damage_size, sizeof(damage_size), 1, fp);
	fwrite(obj->damage, sizeof(utf8_int8_t), damage_size, fp);

	fwrite(&obj->quantity, sizeof(obj->quantity), 1, fp);
	fwrite(&obj->ichar, sizeof(obj->ichar), 1, fp);
	fwrite(&obj->kill_exp, sizeof(obj->kill_exp), 1, fp);
	fwrite(&obj->is_protected, sizeof(obj->is_protected), 1, fp);
	fwrite(&obj->is_cursed, sizeof(obj->is_cursed), 1, fp);
	fwrite(&obj->class, sizeof(obj->class), 1, fp);
	fwrite(&obj->identified, sizeof(obj->identified), 1, fp);
	fwrite(&obj->which_kind, sizeof(obj->which_kind), 1, fp);
	fwrite(&obj->o_row, sizeof(obj->o_row), 1, fp);
	fwrite(&obj->o_col, sizeof(obj->o_col), 1, fp);
	fwrite(&obj->o, sizeof(obj->o), 1, fp);
	fwrite(&obj->row, sizeof(obj->row), 1, fp);
	fwrite(&obj->col, sizeof(obj->col), 1, fp);
	fwrite(&obj->d_enchant, sizeof(obj->d_enchant), 1, fp);
	fwrite(&obj->quiver, sizeof(obj->quiver), 1, fp);
	fwrite(&obj->trow, sizeof(obj->trow), 1, fp);
	fwrite(&obj->tcol, sizeof(obj->tcol), 1, fp);
	fwrite(&obj->hit_enchant, sizeof(obj->hit_enchant), 1, fp);
	fwrite(&obj->what_is, sizeof(obj->what_is), 1, fp);
	fwrite(&obj->picked_up, sizeof(obj->picked_up), 1, fp);
	fwrite(&obj->in_use_flags, sizeof(obj->in_use_flags), 1, fp);
}

/*
 * read_obj
 * オブジェクトの情報をファイルから読み込む
 */
void
read_obj(object *obj, FILE *fp)
{
	fread(&obj->m_flags, sizeof(obj->m_flags), 1, fp);

	int32_t damage_size = 0;
	fread(&damage_size, sizeof(damage_size), 1, fp);
	obj->damage = malloc(damage_size * sizeof(utf8_int8_t));
	fread(obj->damage, sizeof(utf8_int8_t), damage_size, fp);

	fread(&obj->quantity, sizeof(obj->quantity), 1, fp);
	fread(&obj->ichar, sizeof(obj->ichar), 1, fp);
	fread(&obj->kill_exp, sizeof(obj->kill_exp), 1, fp);
	fread(&obj->is_protected, sizeof(obj->is_protected), 1, fp);
	fread(&obj->is_cursed, sizeof(obj->is_cursed), 1, fp);
	fread(&obj->class, sizeof(obj->class), 1, fp);
	fread(&obj->identified, sizeof(obj->identified), 1, fp);
	fread(&obj->which_kind, sizeof(obj->which_kind), 1, fp);
	fread(&obj->o_row, sizeof(obj->o_row), 1, fp);
	fread(&obj->o_col, sizeof(obj->o_col), 1, fp);
	fread(&obj->o, sizeof(obj->o), 1, fp);
	fread(&obj->row, sizeof(obj->row), 1, fp);
	fread(&obj->col, sizeof(obj->col), 1, fp);
	fread(&obj->d_enchant, sizeof(obj->d_enchant), 1, fp);
	fread(&obj->quiver, sizeof(obj->quiver), 1, fp);
	fread(&obj->trow, sizeof(obj->trow), 1, fp);
	fread(&obj->tcol, sizeof(obj->tcol), 1, fp);
	fread(&obj->hit_enchant, sizeof(obj->hit_enchant), 1, fp);
	fread(&obj->what_is, sizeof(obj->what_is), 1, fp);
	fread(&obj->picked_up, sizeof(obj->picked_up), 1, fp);
	fread(&obj->in_use_flags, sizeof(obj->in_use_flags), 1, fp);
	obj->next_object = NULL;
}