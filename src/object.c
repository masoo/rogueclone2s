/*
 * object.c
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

#include "display.h"
#include "invent.h"
#include "machdep.h"
#include "message.h"
#include "monster.h"
#include "object.h"
#include "pack.h"
#include "random.h"
#include "ring.h"
#include "rogue.h"
#include "room.h"
#include "save.h"

object level_objects;
unsigned short dungeon[ROGUE_LINES][ROGUE_COLUMNS];
short foods = 0;
short party_counter;
object *free_list = (object *)0;

char *fruit = mesg[333];

fighter rogue = {
	0, 0,    /* armor, weapon */
	0, 0,    /* rings */
	INIT_HP, /* Hp current */
	INIT_HP, /* Hp max */
	16, 16,  /* Str */
	{0},     /* pack */
	0,       /* gold */
	1, 0,    /* exp, exp_points */
	0, 0,    /* row, col */
	'@',     /* char */
	1250     /* moves */
};

utf8_int8_t *po_color[POTIONS] = {mesg[334], mesg[335], mesg[336], mesg[337],
	mesg[338], mesg[339], mesg[340], mesg[341], mesg[342], mesg[343],
	mesg[344], mesg[345], mesg[346], mesg[347]};

utf8_int8_t po_title[MAX_POTION_QUANTITY][MAX_MESG_BUFFER_SIZE],
    sc_title[SCROLS][MAX_MESG_BUFFER_SIZE],
    wa_title[MAX_WEAPON_QUANTITY][MAX_MESG_BUFFER_SIZE],
    ri_title[MAX_RING_QUANTITY][MAX_MESG_BUFFER_SIZE];

id id_potions[POTIONS] = {
	{100,  po_title[0], mesg[348], 0},
        {250,  po_title[1], mesg[349], 0},
	{100,  po_title[2], mesg[350], 0},
        {200,  po_title[3], mesg[351], 0},
	{ 10,  po_title[4], mesg[352], 0},
        {300,  po_title[5], mesg[353], 0},
	{ 10,  po_title[6], mesg[354], 0},
        { 25,  po_title[7], mesg[355], 0},
	{100,  po_title[8], mesg[356], 0},
        {100,  po_title[9], mesg[357], 0},
	{ 10, po_title[10], mesg[358], 0},
        { 80, po_title[11], mesg[359], 0},
	{150, po_title[12], mesg[360], 0},
        {145, po_title[13], mesg[361], 0}
};

id id_scrolls[SCROLS] = {
	{505,  sc_title[0], mesg[362], 0},
        {200,  sc_title[1], mesg[363], 0},
	{235,  sc_title[2], mesg[364], 0},
        {235,  sc_title[3], mesg[365], 0},
	{175,  sc_title[4], mesg[366], 0},
        {190,  sc_title[5], mesg[367], 0},
	{ 25,  sc_title[6], mesg[368], 0},
        {610,  sc_title[7], mesg[369], 0},
	{210,  sc_title[8], mesg[370], 0},
        {100,  sc_title[9], mesg[371], 0},
	{ 25, sc_title[10], mesg[372], 0},
        {180, sc_title[11], mesg[373], 0}
};
id id_weapons[WEAPONS] = {
	{150, mesg[374], "", 0},
        {  8, mesg[375], "", 0},
        { 15, mesg[376], "", 0},
	{ 27, mesg[377], "", 0},
        { 35, mesg[378], "", 0},
        {360, mesg[379], "", 0},
	{470, mesg[380], "", 0},
        {580, mesg[381], "", 0}
};

id id_armors[ARMORS] = {
	{300, mesg[382], "", (UNIDENTIFIED)},
	{300, mesg[383], "", (UNIDENTIFIED)},
	{400, mesg[384], "", (UNIDENTIFIED)},
	{500, mesg[385], "", (UNIDENTIFIED)},
	{600, mesg[386], "", (UNIDENTIFIED)},
	{600, mesg[387], "", (UNIDENTIFIED)},
	{700, mesg[388], "", (UNIDENTIFIED)}
};

id id_wands[WANDS] = {
	{25, wa_title[0], mesg[389], 0},
        {50, wa_title[1], mesg[390], 0},
	{45, wa_title[2], mesg[391], 0},
        { 8, wa_title[3], mesg[392], 0},
	{55, wa_title[4], mesg[393], 0},
        { 2, wa_title[5], mesg[394], 0},
	{25, wa_title[6], mesg[395], 0},
        {20, wa_title[7], mesg[396], 0},
	{20, wa_title[8], mesg[397], 0},
        { 0, wa_title[9], mesg[398], 0}
};

id id_rings[RINGS] = {
	{250,  ri_title[0], mesg[399], 0},
        {100,  ri_title[1], mesg[400], 0},
	{255,  ri_title[2], mesg[401], 0},
        {295,  ri_title[3], mesg[402], 0},
	{200,  ri_title[4], mesg[403], 0},
        {250,  ri_title[5], mesg[404], 0},
	{250,  ri_title[6], mesg[405], 0},
        { 25,  ri_title[7], mesg[406], 0},
	{300,  ri_title[8], mesg[407], 0},
        {290,  ri_title[9], mesg[408], 0},
	{270, ri_title[10], mesg[409], 0}
};

extern short cur_level, max_level;
extern short party_room;
extern char *error_file;
extern bool is_wood[];

void
put_objects(void)
{
	short i, n;
	object *obj;

	if (cur_level < max_level) {
		return;
	}
	n = coin_toss() ? get_rand(2, 4) : get_rand(3, 5);
	while (rand_percent(33)) {
		n++;
	}
	if (cur_level == party_counter) {
		make_party();
		party_counter = next_party();
	}
	for (i = 0; i < n; i++) {
		obj = gr_object();
		rand_place(obj);
	}
	put_gold();
}

void
put_gold(void)
{
	short i, j;
	short row, col;
	bool is_maze, is_room;

	for (i = 0; i < MAXROOMS; i++) {
		is_maze = (rooms[i].is_room & R_MAZE) ? 1 : 0;
		is_room = (rooms[i].is_room & R_ROOM) ? 1 : 0;

		if (!(is_room || is_maze)) {
			continue;
		}
		if (is_maze || rand_percent(GOLD_PERCENT)) {
			for (j = 0; j < 50; j++) {
				row = get_rand(rooms[i].top_row + 1,
				    rooms[i].bottom_row - 1);
				col = get_rand(rooms[i].left_col + 1,
				    rooms[i].right_col - 1);
				if ((dungeon[row][col] == FLOOR) ||
				    (dungeon[row][col] == TUNNEL)) {
					plant_gold(row, col, is_maze);
					break;
				}
			}
		}
	}
}

void
plant_gold(short row, short col, bool is_maze)
{
	object *obj;

	obj = alloc_object();
	obj->row = row;
	obj->col = col;
	obj->what_is = GOLD;
	obj->quantity = get_rand((2 * cur_level), (16 * cur_level));
	if (is_maze) {
		obj->quantity += obj->quantity / 2;
	}
	dungeon[row][col] |= OBJECT;
	(void)add_to_pack(obj, &level_objects, 0);
}

void
place_at(object *obj, int row, int col)
{
	obj->row = row;
	obj->col = col;
	dungeon[row][col] |= OBJECT;
	(void)add_to_pack(obj, &level_objects, 0);
}

object *
object_at(object *pack, short row, short col)
{
	object *obj;

	obj = pack->next_object;

	while (obj && ((obj->row != row) || (obj->col != col))) {
		obj = obj->next_object;
	}
	return obj;
}

object *
get_letter_object(int ch)
{
	object *obj;

	obj = rogue.pack.next_object;

	while (obj && (obj->ichar != ch)) {
		obj = obj->next_object;
	}
	return obj;
}

void
free_stuff(object *objlist)
{
	object *obj;

	while (objlist->next_object) {
		obj = objlist->next_object;
		objlist->next_object = objlist->next_object->next_object;
		free_object(obj);
	}
}

char *
name_of(object *obj)
{
	int i;
	static unsigned short wa[] = {SCROL, POTION, WAND, ARMOR, RING, AMULET};
	static char *na[] = {mesg[3], mesg[4], mesg[5], mesg[7], mesg[8],
		mesg[9]};
	if (obj->what_is == WEAPON) {
		return id_weapons[obj->which_kind].title;
	}
	if (obj->what_is == FOOD) {
		return (obj->which_kind == RATION) ? mesg[2] : fruit;
	}
	for (i = 0; i < 6; i++) {
		if (obj->what_is == wa[i]) {
			return na[i];
		}
	}
	return mesg[80];
}

object *
gr_object(void)
{
	object *obj;

	obj = alloc_object();

	if (foods < (cur_level / 3)) {
		obj->what_is = FOOD;
		foods++;
	} else {
		obj->what_is = gr_what_is();
	}
	switch (obj->what_is) {
	case SCROL:
		gr_scroll(obj);
		break;
	case POTION:
		gr_potion(obj);
		break;
	case WEAPON:
		gr_weapon(obj, 1);
		break;
	case ARMOR:
		gr_armor(obj, 1);
		break;
	case WAND:
		gr_wand(obj);
		break;
	case FOOD:
		get_food(obj, 0);
		break;
	case RING:
		gr_ring(obj, 1);
		break;
	}
	return obj;
}

unsigned short
gr_what_is(void)
{
	short percent;
	int i;
	static short per[] = {30, 60, 64, 74, 83, 88, 91};
	static unsigned short ret[] = {SCROL, POTION, WAND, WEAPON, ARMOR, FOOD,
		RING};

	percent = get_rand(1, 91);

	for (i = 0;; i++) {
		if (percent <= per[i])
			return ret[i];
	}
}

void
gr_scroll(object *obj)
{
	short percent;
	int i;
	static short per[SCROLS] = {5, 11, 16, 21, 36, 44, 51, 56, 65, 74, 80,
		85};

	percent = get_rand(0, 85);
	obj->what_is = SCROL;
	for (i = 0;; i++) {
		if (percent <= per[i]) {
			obj->which_kind = i;
			return;
		}
	}
}

void
gr_potion(object *obj)
{
	short percent;
	int i;
	static short per[POTIONS] = {10, 20, 30, 40, 50, 55, 65, 75, 85, 95,
		105, 110, 114, 118};

	percent = get_rand(1, 118);
	obj->what_is = POTION;
	for (i = 0; i < POTIONS; i++) {
		if (percent <= per[i]) {
			obj->which_kind = i;
			return;
		}
	}
}

void
gr_weapon(object *obj, int assign_wk)
{
	short i;
	short percent;
	short blessing, increment;
	static char *da[WEAPONS] = {"1d1", "1d1", "1d2", "1d3", "1d4", "2d3",
		"3d4", "4d5"};

	obj->what_is = WEAPON;
	if (assign_wk) {
		obj->which_kind = get_rand(0, (WEAPONS - 1));
	}
	if ((i = obj->which_kind) == ARROW || i == DAGGER || i == SHURIKEN ||
	    i == DART) {
		obj->quantity = get_rand(3, 15);
		obj->quiver = get_rand(0, 126);
	} else {
		obj->quantity = 1;
	}
	obj->hit_enchant = obj->d_enchant = 0;

	percent = get_rand(1, 96);
	blessing = get_rand(1, 3);

	if (percent <= 16) {
		increment = 1;
	} else if (percent <= 32) {
		increment = -1;
		obj->is_cursed = 1;
	}
	if (percent <= 32) {
		for (i = 0; i < blessing; i++) {
			if (coin_toss()) {
				obj->hit_enchant += increment;
			} else {
				obj->d_enchant += increment;
			}
		}
	}
	obj->damage = da[obj->which_kind];
}

void
gr_armor(object *obj, int assign_wk)
{ /* by Yasha */
	short percent;
	short blessing;

	obj->what_is = ARMOR;
	if (assign_wk) /* by Yasha */
		obj->which_kind = get_rand(0, (ARMORS - 1));
	obj->class = obj->which_kind + 2;
	if ((obj->which_kind == PLATE) || (obj->which_kind == SPLINT)) {
		obj->class --;
	}
	obj->is_protected = 0;
	obj->d_enchant = 0;

	percent = get_rand(1, 100);
	blessing = get_rand(1, 3);

	if (percent <= 16) {
		obj->is_cursed = 1;
		obj->d_enchant -= blessing;
	} else if (percent <= 33) {
		obj->d_enchant += blessing;
	}
}

void
gr_wand(object *obj)
{
	obj->what_is = WAND;
	obj->which_kind = get_rand(0, (WANDS - 1));
	if (obj->which_kind == MAGIC_MISSILE) {
		obj->class = get_rand(6, 12);
	} else if (obj->which_kind == CANCELLATION) {
		obj->class = get_rand(5, 9);
	} else {
		obj->class = get_rand(3, 6);
	}
}

void
get_food(object *obj, bool force_ration)
{
	obj->what_is = FOOD;

	if (force_ration || rand_percent(80)) {
		obj->which_kind = RATION;
	} else {
		obj->which_kind = FRUIT;
	}
}

void
put_stairs(void)
{
	short row, col;

	gr_row_col(&row, &col, (FLOOR | TUNNEL));
	dungeon[row][col] |= STAIRS;
}

int
get_armor_class(object *obj)
{
	if (obj) {
		return (obj->class + obj->d_enchant);
	}
	return (0);
}

object *
alloc_object(void)
{
	object *obj;

	if (free_list) {
		obj = free_list;
		free_list = free_list->next_object;
	} else if (!(obj = (object *)md_malloc(sizeof(object)))) {
		message("メモリーが足りません。 ゲームをセーブします。", 0);
		save_into_file(error_file);
	}
	obj->quantity = 1;
	obj->ichar = 'L';
	obj->picked_up = obj->is_cursed = 0;
	obj->in_use_flags = NOT_USED;
	obj->identified = UNIDENTIFIED;
	obj->damage = "1d1";
	return obj;
}

void
free_object(object *obj)
{
	obj->next_object = free_list;
	free_list = obj;
}

void
make_party(void)
{
	short n;

	party_room = gr_room();

	n = rand_percent(99) ? party_objects(party_room) : 11;
	if (rand_percent(99)) {
		party_monsters(party_room, n);
	}
}

void
show_objects(void)
{
	object *obj;
	short mc, rc, row, col;
	object *monster;

	obj = level_objects.next_object;

	while (obj) {
		row = obj->row;
		col = obj->col;

		rc = get_mask_char(obj->what_is);

		if (dungeon[row][col] & MONSTER) {
			if ((monster = object_at(&level_monsters, row, col))) {
				monster->trail_char = rc;
			}
		}
		mc = mvinch_rogue(row, col);
		if (((mc < 'A') || (mc > 'Z')) &&
		    ((row != rogue.row) || (col != rogue.col))) {
			mvaddch_rogue(row, col, rc);
		}
		obj = obj->next_object;
	}

	monster = level_monsters.next_object;

	while (monster) {
		if (monster->m_flags & IMITATES) {
			mvaddch_rogue(monster->row, monster->col,
			    monster->disguise);
		}
		monster = monster->next_monster;
	}
}

void
put_amulet(void)
{
	object *obj;

	obj = alloc_object();
	obj->what_is = AMULET;
	rand_place(obj);
}

void
rand_place(object *obj)
{
	short row, col;

	gr_row_col(&row, &col, (FLOOR | TUNNEL));
	place_at(obj, row, col);
}

void
new_object_for_wizard(void)
{
	short ch, max = 0; /* 未初期化変数の使用の Warning の対策で 0 を不可 */
	object *obj;
	char buf[80];

	if (pack_count((object *)0) >= MAX_PACK_COUNT) {
		message(mesg[81], 0);
		return;
	}
	message(mesg[82], 0);

	while (r_index("!?:)]=/,\033", (ch = rgetchar()), 0) == -1) {
		sound_bell();
	}
	check_message();
	if (ch == '\033') {
		return;
	}

	obj = alloc_object();

	switch (ch) {
	case '!':
		obj->what_is = POTION;
		max = POTIONS - 1;
		break;
	case '?':
		obj->what_is = SCROL;
		max = SCROLS - 1;
		break;
	case ',':
		obj->what_is = AMULET;
		break;
	case ':':
		get_food(obj, 0);
		break;
	case ')':
		/*		gr_weapon(obj, 0);*/
		obj->what_is = WEAPON;
		max = WEAPONS - 1;
		break;
	case ']':
		/*		gr_armor(obj);*/
		obj->what_is = ARMOR; /* by Yasha */
		max = ARMORS - 1;
		break;
	case '/':
		gr_wand(obj);
		max = WANDS - 1;
		break;
	case '=':
		max = RINGS - 1;
		obj->what_is = RING;
		break;
	}
	if ((ch != ',') && (ch != ':')) {
		/*		sprintf(buf, mesg[83], name_of(obj));*/
		sprintf(buf, mesg[83],
		    (obj->what_is == WEAPON) /* by Yasha */
			? mesg[84]
			: name_of(obj)); /* by Yasha */
		for (;;) {
			message(buf, 0);
			for (;;) {
				ch = rgetchar();
				if ((ch != LIST && ch != CANCEL && ch < 'a') ||
				    ch > 'a' + max) {
					sound_bell();
				} else {
					break;
				}
			}
			if (ch == LIST) {
				check_message();
				list_object(obj, max);
			} else {
				break;
			}
		}
		check_message();
		if (ch == CANCEL) {
			free_object(obj);
			return;
		}
		obj->which_kind = ch - 'a';
		if (obj->what_is == RING) {
			gr_ring(obj, 0);
		}

		if (obj->what_is == ARMOR) {         /* by Yasha */
			gr_armor(obj, 0);            /* by Yasha */
		} else if (obj->what_is == WEAPON) { /* by Yasha */
			gr_weapon(obj, 0);           /* by Yasha */
		}
	}
	get_desc(obj, buf, 1);
	message(buf, 0);
	(void)add_to_pack(obj, &rogue.pack, 1);
}

void
list_object(object *obj, short max)
{
	short i, j, maxlen, n;
	char descs[ROGUE_LINES][ROGUE_COLUMNS];
	short row, col;
	id *id;
	int weapon_or_armor; /* by Yasha */
#if defined(COLOR)
	char *p;
#endif /* COLOR */
	char *msg = "  ＝スペースを押してください＝";
	short len = 30;

	weapon_or_armor = 0;
	switch (obj->what_is) {
	case ARMOR:
		id = id_armors;
		weapon_or_armor = 1; /* by Yasha */
		break;
	case WEAPON:
		id = id_weapons;
		weapon_or_armor = 1; /* by Yasha */
		break;
	case SCROL:
		id = id_scrolls;
		break;
	case POTION:
		id = id_potions;
		break;
	case WAND:
		id = id_wands;
		break;
	case RING:
		id = id_rings;
		break;
	default:
		return;
	}

	maxlen = len;
	for (i = 0; i <= max; i++) {
#if 1 /* by Yasha */
		sprintf(descs[i], " %c) %s%s", i + 'a',
		    weapon_or_armor ? id[i].title : id[i].real,
		    weapon_or_armor ? "" : name_of(obj));
#else
		sprintf(descs[i], " %c) %s%s", i + 'a', id[i].real,
		    name_of(obj));
#endif
		if ((n = strlen(descs[i])) > maxlen) {
			maxlen = n;
		}
	}
	(void)strcpy(descs[i++], msg);

	col = ROGUE_COLUMNS - (maxlen + 2);
	for (row = 0; row < i; row++) {
		if (row > 0) {
			for (j = col; j < ROGUE_COLUMNS; j++) {
				descs[row - 1][j - col] = mvinch_rogue(row, j);
			}
			descs[row - 1][j - col] = 0;
		}
		mvaddstr_rogue(row, col, descs[row]);
		clrtoeol();
	}
	refresh();
	wait_for_ack();

	move(0, 0);
	clrtoeol();
#if defined(COLOR)
	for (j = 1; j < i; j++) {
		move(j, col);
		for (p = descs[j - 1]; *p; p++) {
			addch_rogue(*p);
		}
	}
#else  /* not COLOR */
	for (j = 1; j < i; j++) {
		mvaddstr_rogue(j, col, descs[j - 1]);
	}
#endif /* not COLOR */
}

int
next_party(void)
{
	int n;

	n = cur_level;
	while (n % PARTY_TIME) {
		n++;
	}
	return (get_rand((n + 1), (n + PARTY_TIME)));
}
