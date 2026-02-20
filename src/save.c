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
#include <stdint.h>
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

#define SAVE_MAGIC "RC2S"
#define SAVE_VERSION 1

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

/*
 * r_read
 * ファイルからバッファに読み込む
 */
void
r_read(FILE *fp, char *buf, int n)
{
	if (fread(buf, sizeof(char), n, fp) != n) {
		clean_up("ファイルが読めません。");
	}
}

/*
 * r_write
 * バッファの内容をファイルに書き込む
 */
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

/* リトルエンディアンでの型付き書き込み */

static void
write_bool(FILE *fp, bool val)
{
	uint8_t v = val ? 1 : 0;
	r_write(fp, (char *)&v, 1);
}

static void
write_int8(FILE *fp, int8_t val)
{
	r_write(fp, (char *)&val, 1);
}

static void
write_uint8(FILE *fp, uint8_t val)
{
	r_write(fp, (char *)&val, 1);
}

static void
write_int16(FILE *fp, int16_t val)
{
	uint8_t buf[2];
	uint16_t u;
	memcpy(&u, &val, 2);
	buf[0] = u & 0xFF;
	buf[1] = (u >> 8) & 0xFF;
	r_write(fp, (char *)buf, 2);
}

static void
write_uint16(FILE *fp, uint16_t val)
{
	uint8_t buf[2];
	buf[0] = val & 0xFF;
	buf[1] = (val >> 8) & 0xFF;
	r_write(fp, (char *)buf, 2);
}

static void
write_int32(FILE *fp, int32_t val)
{
	uint8_t buf[4];
	uint32_t u;
	memcpy(&u, &val, 4);
	buf[0] = u & 0xFF;
	buf[1] = (u >> 8) & 0xFF;
	buf[2] = (u >> 16) & 0xFF;
	buf[3] = (u >> 24) & 0xFF;
	r_write(fp, (char *)buf, 4);
}

static void
write_uint32(FILE *fp, uint32_t val)
{
	uint8_t buf[4];
	buf[0] = val & 0xFF;
	buf[1] = (val >> 8) & 0xFF;
	buf[2] = (val >> 16) & 0xFF;
	buf[3] = (val >> 24) & 0xFF;
	r_write(fp, (char *)buf, 4);
}

/* リトルエンディアンでの型付き読み込み */

static bool
read_bool(FILE *fp)
{
	uint8_t v;
	r_read(fp, (char *)&v, 1);
	return v != 0;
}

static int8_t
read_int8(FILE *fp)
{
	int8_t val;
	r_read(fp, (char *)&val, 1);
	return val;
}

static uint8_t
read_uint8(FILE *fp)
{
	uint8_t val;
	r_read(fp, (char *)&val, 1);
	return val;
}

static int16_t
read_int16(FILE *fp)
{
	uint8_t buf[2];
	r_read(fp, (char *)buf, 2);
	uint16_t u = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
	int16_t val;
	memcpy(&val, &u, 2);
	return val;
}

static uint16_t
read_uint16(FILE *fp)
{
	uint8_t buf[2];
	r_read(fp, (char *)buf, 2);
	return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

static int32_t
read_int32(FILE *fp)
{
	uint8_t buf[4];
	r_read(fp, (char *)buf, 4);
	uint32_t u = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
	    ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
	int32_t val;
	memcpy(&val, &u, 4);
	return val;
}

static uint32_t
read_uint32(FILE *fp)
{
	uint8_t buf[4];
	r_read(fp, (char *)buf, 4);
	return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
	    ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

/* 構造体のフィールド別シリアライズ */

static void
write_fighter(FILE *fp, fighter *f)
{
	write_int16(fp, f->hp_current);
	write_int16(fp, f->hp_max);
	write_int16(fp, f->str_current);
	write_int16(fp, f->str_max);
	write_int32(fp, (int32_t)f->gold);
	write_int16(fp, f->exp);
	write_int32(fp, (int32_t)f->exp_points);
	write_int16(fp, f->row);
	write_int16(fp, f->col);
	write_int16(fp, f->fchar);
	write_int16(fp, f->moves_left);
}

static void
read_fighter(FILE *fp, fighter *f)
{
	f->armor = NULL;
	f->weapon = NULL;
	f->left_ring = NULL;
	f->right_ring = NULL;
	f->hp_current = read_int16(fp);
	f->hp_max = read_int16(fp);
	f->str_current = read_int16(fp);
	f->str_max = read_int16(fp);
	f->pack.next_object = NULL;
	f->gold = read_int32(fp);
	f->exp = read_int16(fp);
	f->exp_points = read_int32(fp);
	f->row = read_int16(fp);
	f->col = read_int16(fp);
	f->fchar = read_int16(fp);
	f->moves_left = read_int16(fp);
}

static void
write_trap(FILE *fp, trap *t)
{
	write_int16(fp, t->trap_type);
	write_int16(fp, t->trap_row);
	write_int16(fp, t->trap_col);
}

static void
read_trap(FILE *fp, trap *t)
{
	t->trap_type = read_int16(fp);
	t->trap_row = read_int16(fp);
	t->trap_col = read_int16(fp);
}

static void
write_door(FILE *fp, door *d)
{
	write_int16(fp, d->oth_room);
	write_int16(fp, d->oth_row);
	write_int16(fp, d->oth_col);
	write_int16(fp, d->door_row);
	write_int16(fp, d->door_col);
}

static void
read_door(FILE *fp, door *d)
{
	d->oth_room = read_int16(fp);
	d->oth_row = read_int16(fp);
	d->oth_col = read_int16(fp);
	d->door_row = read_int16(fp);
	d->door_col = read_int16(fp);
}

static void
write_room(FILE *fp, room *r)
{
	write_int8(fp, r->bottom_row);
	write_int8(fp, r->right_col);
	write_int8(fp, r->left_col);
	write_int8(fp, r->top_row);
	for (int k = 0; k < 4; k++)
		write_door(fp, &r->doors[k]);
	write_uint16(fp, r->is_room);
}

static void
read_room(FILE *fp, room *r)
{
	r->bottom_row = read_int8(fp);
	r->right_col = read_int8(fp);
	r->left_col = read_int8(fp);
	r->top_row = read_int8(fp);
	for (int k = 0; k < 4; k++)
		read_door(fp, &r->doors[k]);
	r->is_room = read_uint16(fp);
}

/*
 * save_game
 * ゲームのセーブを行う
 */
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

/*
 * save_into_file
 * 指定ファイルにゲーム状態を書き出す
 */
void
save_into_file(char *sfile)
{
	FILE *fp;
	int file_id;
	char name_buffer[ROGUE_PATH_MAX];
	char *hptr;
	rogue_time rt_buf;

	if (*org_dir) {
		if (chdir(org_dir) == -1)
			clean_up("ディレクトリを変更できません。");
	}
	if (sfile[0] == '~') {
		if ((hptr = md_ghome())) {
			(void)snprintf(name_buffer, sizeof(name_buffer), "%s%s",
			    hptr, sfile + 1);
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
	r_write(fp, SAVE_MAGIC, 4);
	write_uint16(fp, SAVE_VERSION);
	(void)xxx(1);
	write_bool(fp, detect_monster);
	write_int16(fp, cur_level);
	write_int16(fp, max_level);
	write_string(hunger_str, fp);
	write_string(login_name, fp);
	write_int16(fp, party_room);
	write_int16(fp, party_counter);
	write_pack(&level_monsters, fp);
	write_pack(&level_objects, fp);
	write_int32(fp, file_id);
	rw_dungeon(fp, 1);
	write_int16(fp, foods);
	write_fighter(fp, &rogue);
	write_pack(&rogue.pack, fp);
	rw_id(id_potions, fp, POTIONS, 1);
	rw_id(id_scrolls, fp, SCROLS, 1);
	rw_id(id_wands, fp, WANDS, 1);
	rw_id(id_rings, fp, RINGS, 1);
	for (short i = 0; i < MAX_TRAPS; i++)
		write_trap(fp, &traps[i]);
	for (short i = 0; i < WANDS; i++)
		write_bool(fp, is_wood[i]);
	write_int16(fp, cur_room);
	rw_rooms(fp, 1);
	write_bool(fp, being_held);
	write_int16(fp, bear_trap);
	write_int16(fp, halluc);
	write_int16(fp, blind);
	write_int16(fp, confused);
	write_int16(fp, levitate);
	write_int16(fp, haste_self);
	write_bool(fp, see_invisible);
	write_bool(fp, detect_monster);
	write_bool(fp, wizard);
	write_bool(fp, score_only);
	write_int16(fp, m_moves);
	md_gct(&rt_buf);
	rt_buf.second += 10; /* allow for some processing time */
	write_int16(fp, rt_buf.year);
	write_int16(fp, rt_buf.month);
	write_int16(fp, rt_buf.day);
	write_int16(fp, rt_buf.hour);
	write_int16(fp, rt_buf.minute);
	write_int16(fp, rt_buf.second);
	fclose(fp);

	if (write_failed) {
		(void)md_df(sfile); /* delete file */
	} else {
		clean_up("");
	}

err_return:;
	if (game_dir && *game_dir) {
		if (chdir(game_dir) == -1)
			clean_up("ディレクトリを変更できません。");
	}
}

/*
 * restore
 * セーブファイルからゲーム状態を復元する
 */
void
restore(char *fname)
{
	FILE *fp = {
		NULL}; /* 未初期化変数の使用の警告除去のため NULL で初期化 */
	rogue_time saved_time, mod_time;
	char buf[4];
	char tbuf[40];
	int new_file_id, saved_file_id;

	if (*org_dir) {
		if (chdir(org_dir) == -1)
			clean_up("ディレクトリを変更できません。");
	}

	if (((new_file_id = md_get_file_id(fname)) == -1) ||
	    ((fp = fopen(fname, "rb")) == NULL)) {
		clean_up("ファイルがオープンできませんでした。");
	}
	if (md_link_count(fname) > 1) {
		clean_up("ファイルはリンクされています。");
	}
	char magic[4];
	r_read(fp, magic, 4);
	if (memcmp(magic, SAVE_MAGIC, 4) != 0) {
		clean_up("セーブファイルの形式が不正です。");
	}
	uint16_t version = read_uint16(fp);
	if (version != SAVE_VERSION) {
		clean_up("セーブファイルのバージョンが不正です。");
	}
	(void)xxx(1);
	detect_monster = read_bool(fp);
	cur_level = read_int16(fp);
	max_level = read_int16(fp);
	read_string(hunger_str, fp);

	(void)strcpy(tbuf, login_name);
	read_string(login_name, fp);
	if (strcmp(tbuf, login_name)) {
		clean_up("セーブファイルの持ち主が違います。");
	}

	party_room = read_int16(fp);
	party_counter = read_int16(fp);
	read_pack(&level_monsters, fp, 0);
	read_pack(&level_objects, fp, 0);
	saved_file_id = read_int32(fp);
	if (new_file_id != saved_file_id) {
		clean_up("これは元のセーブファイルではありません。");
	}
	rw_dungeon(fp, 0);
	foods = read_int16(fp);
	read_fighter(fp, &rogue);
	read_pack(&rogue.pack, fp, 1);
	rw_id(id_potions, fp, POTIONS, 0);
	rw_id(id_scrolls, fp, SCROLS, 0);
	rw_id(id_wands, fp, WANDS, 0);
	rw_id(id_rings, fp, RINGS, 0);
	for (short i = 0; i < MAX_TRAPS; i++)
		read_trap(fp, &traps[i]);
	for (short i = 0; i < WANDS; i++)
		is_wood[i] = read_bool(fp);
	cur_room = read_int16(fp);
	rw_rooms(fp, 0);
	being_held = read_bool(fp);
	bear_trap = read_int16(fp);
	halluc = read_int16(fp);
	blind = read_int16(fp);
	confused = read_int16(fp);
	levitate = read_int16(fp);
	haste_self = read_int16(fp);
	see_invisible = read_bool(fp);
	detect_monster = read_bool(fp);
	wizard = read_bool(fp);
	score_only = read_bool(fp);
	m_moves = read_int16(fp);
	saved_time.year = read_int16(fp);
	saved_time.month = read_int16(fp);
	saved_time.day = read_int16(fp);
	saved_time.hour = read_int16(fp);
	saved_time.minute = read_int16(fp);
	saved_time.second = read_int16(fp);

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
		if (chdir(game_dir) == -1)
			clean_up("ディレクトリを変更できません。");
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
	t.damage = "";
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
			for (j = 0; j < ROGUE_COLUMNS; j++) {
				write_uint16(fp, dungeon[i][j]);
			}
			for (j = 0; j < ROGUE_COLUMNS; j++) {
				buf[j] = mvinch_rogue(i, j);
			}
			r_write(fp, buf, ROGUE_COLUMNS);
		}
		return;
	} else { // 読み込み時実行
		for (i = 0; i < ROGUE_LINES; i++) {
			for (j = 0; j < ROGUE_COLUMNS; j++) {
				dungeon[i][j] = read_uint16(fp);
			}
			r_read(fp, buf, ROGUE_COLUMNS);

			if (i < ROGUE_LINES - 1) {
				for (j = 0; j < ROGUE_COLUMNS; j++) {
					if (i < MIN_ROW) {
						mvaddch(i, j,
						    (unsigned char)buf[j]);
					} else {
						mvaddch_rogue(i, j,
						    (unsigned char)buf[j]);
					}
				}
			} else {
				print_stats(STAT_ALL);
			}
		}
		return;
	}
}

/*
 * rw_id
 * 識別テーブルの読み書きを行う
 */
void
rw_id(id id_table[], FILE *fp, int n, bool wr)
{
	short i;

	for (i = 0; i < n; i++) {
		if (wr) {
			write_int16(fp, id_table[i].value);
			write_uint16(fp, id_table[i].id_status);
			write_string(id_table[i].title, fp);
		} else {
			id_table[i].value = read_int16(fp);
			id_table[i].id_status = read_uint16(fp);
			read_string(id_table[i].title, fp);
		}
	}
}

/*
 * write_string
 * 文字列をファイルに書き込む
 */
void
write_string(char *s, FILE *fp)
{
	short n;

	n = strlen(s) + 1;
	xxxx(s, n);
	write_int16(fp, n);
	r_write(fp, s, n);
}

/*
 * read_string
 * 文字列をファイルから読み込む
 */
void
read_string(char *s, FILE *fp)
{
	short n;

	n = read_int16(fp);
	r_read(fp, s, n);
	xxxx(s, n);
}

/*
 * rw_rooms
 * 部屋情報の読み書きを行う
 */
void
rw_rooms(FILE *fp, bool rw)
{
	short i;

	for (i = 0; i < MAXROOMS; i++) {
		rw ? write_room(fp, &rooms[i])
		   : read_room(fp, &rooms[i]);
	}
}

/*
 * has_been_touched
 * セーブファイルが改竄されていないか判定する
 */
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
	write_uint32(fp, obj->m_flags);

	int32_t damage_size = utf8size(obj->damage);
	write_int32(fp, damage_size);
	r_write(fp, (char *)obj->damage, damage_size * sizeof(utf8_int8_t));

	write_int16(fp, obj->quantity);
	write_int8(fp, obj->ichar);
	write_int16(fp, obj->kill_exp);
	write_int8(fp, obj->is_protected);
	write_int8(fp, obj->is_cursed);
	write_int8(fp, obj->class);
	write_int8(fp, obj->identified);
	write_uint8(fp, obj->which_kind);
	write_int8(fp, obj->o_row);
	write_int8(fp, obj->o_col);
	write_int8(fp, obj->o);
	write_int8(fp, obj->row);
	write_int8(fp, obj->col);
	write_int8(fp, obj->d_enchant);
	write_int8(fp, obj->quiver);
	write_int8(fp, obj->trow);
	write_int8(fp, obj->tcol);
	write_int8(fp, obj->hit_enchant);
	write_uint8(fp, obj->what_is);
	write_int8(fp, obj->picked_up);
	write_uint8(fp, obj->in_use_flags);
}

/*
 * read_obj
 * オブジェクトの情報をファイルから読み込む
 */
void
read_obj(object *obj, FILE *fp)
{
	obj->m_flags = read_uint32(fp);

	int32_t damage_size = read_int32(fp);
	obj->damage = malloc(damage_size * sizeof(utf8_int8_t));
	r_read(fp, (char *)obj->damage, damage_size * sizeof(utf8_int8_t));

	obj->quantity = read_int16(fp);
	obj->ichar = read_int8(fp);
	obj->kill_exp = read_int16(fp);
	obj->is_protected = read_int8(fp);
	obj->is_cursed = read_int8(fp);
	obj->class = read_int8(fp);
	obj->identified = read_int8(fp);
	obj->which_kind = read_uint8(fp);
	obj->o_row = read_int8(fp);
	obj->o_col = read_int8(fp);
	obj->o = read_int8(fp);
	obj->row = read_int8(fp);
	obj->col = read_int8(fp);
	obj->d_enchant = read_int8(fp);
	obj->quiver = read_int8(fp);
	obj->trow = read_int8(fp);
	obj->tcol = read_int8(fp);
	obj->hit_enchant = read_int8(fp);
	obj->what_is = read_uint8(fp);
	obj->picked_up = read_int8(fp);
	obj->in_use_flags = read_uint8(fp);
	obj->next_object = NULL;
}
