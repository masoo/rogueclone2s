/*
 * pack.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#include "rogue.h"

char *curse_message = mesg[85];

object *
add_to_pack(obj, pack, condense)
object *obj, *pack;
{
	object *op, *p;
	
	if (condense) {
		if (op = check_duplicate(obj, pack)) {
			free_object(obj);
			return(op);
		} else {
			obj->ichar = next_avail_ichar();
		}
	}
#ifndef ORIGINAL
	for (op = pack; op->next_object; op = op->next_object) {
		if (op->next_object->what_is > obj->what_is) {
			p = op->next_object;
			op->next_object = obj;
			obj->next_object = p;
			return obj;
		}
	}
	op->next_object = obj;
	obj->next_object = 0;
	return obj;
#else
	if (pack->next_object == 0) {
		pack->next_object = obj;
	} else {
		op = pack->next_object;

		while (op->next_object) {
			op = op->next_object;
		}
		op->next_object = obj;
	}
	obj->next_object = 0;
	return(obj);
#endif
}

take_from_pack(obj, pack)
object *obj, *pack;
{
	while (pack->next_object != obj) {
		pack = pack->next_object;
	}
	pack->next_object = pack->next_object->next_object;
}

object *
pick_up(row, col, status)
short *status;
{
	object *obj;

	obj = object_at(&level_objects, row, col);
	*status = 1;

	if ((obj->what_is == SCROL) && (obj->which_kind == SCARE_MONSTER) &&
		obj->picked_up) {
		message(mesg[86], 0);
		dungeon[row][col] &= (~OBJECT);
		vanish(obj, 0, &level_objects);
		*status = 0;
		if (id_scrolls[SCARE_MONSTER].id_status == UNIDENTIFIED) {
			id_scrolls[SCARE_MONSTER].id_status = IDENTIFIED;
		}
		return(0);
	}
	if (obj->what_is == GOLD) {
		rogue.gold += obj->quantity;
		dungeon[row][col] &= ~(OBJECT);
		take_from_pack(obj, &level_objects);
		print_stats(STAT_GOLD);
		return(obj);	/* obj will be free_object()ed in one_move_rogue() */
	}
	if (pack_count(obj) >= MAX_PACK_COUNT) {
		message(mesg[87], 1);
		return(0);
	}
	dungeon[row][col] &= ~(OBJECT);
	take_from_pack(obj, &level_objects);
	obj = add_to_pack(obj, &rogue.pack, 1);
	obj->picked_up = 1;
	return(obj);
}

drop()
{
	object *obj, *new;
	short ch;
	char desc[DCOLS];

	if (dungeon[rogue.row][rogue.col] & (OBJECT | STAIRS | TRAP)) {
		message(mesg[88], 0);
		return;
	}
	if (!rogue.pack.next_object) {
		message(mesg[89], 0);
		return;
	}
	if ((ch = pack_letter(mesg[90], ALL_OBJECTS)) == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(mesg[91], 0);
		return;
	}
	if (obj->in_use_flags & BEING_WIELDED) {
		if (obj->is_cursed) {
			message(curse_message, 0);
			return;
		}
		unwield(rogue.weapon);
	} else if (obj->in_use_flags & BEING_WORN) {
		if (obj->is_cursed) {
			message(curse_message, 0);
			return;
		}
		mv_aquatars();
		unwear(rogue.armor);
		print_stats(STAT_ARMOR);
	} else if (obj->in_use_flags & ON_EITHER_HAND) {
		if (obj->is_cursed) {
			message(curse_message, 0);
			return;
		}
		un_put_on(obj);
	}
	obj->row = rogue.row;
	obj->col = rogue.col;

	if ((obj->quantity > 1) && (obj->what_is != WEAPON)) {
		obj->quantity--;
		new = alloc_object();
		*new = *obj;
		new->quantity = 1;
		obj = new;
	} else {
		obj->ichar = 'L';
		take_from_pack(obj, &rogue.pack);
	}
	place_at(obj, rogue.row, rogue.col);
#ifdef JAPAN
	get_desc(obj, desc, 0);
	(void) strcat(desc, mesg[92]);
#else
	(void) strcpy(desc, mesg[92]);
	get_desc(obj, desc+strlen(mesg[92]), 0);
#endif
	message(desc, 0);
	(void) reg_move();
}

object *
check_duplicate(obj, pack)
object *obj, *pack;
{
	object *op;

	if (!(obj->what_is & (WEAPON | FOOD | SCROL | POTION))) {
		return(0);
	}
	if ((obj->what_is == FOOD) && (obj->which_kind == FRUIT)) {
		return(0);
	}
	op = pack->next_object;

	while (op) {
		if ((op->what_is == obj->what_is) && 
			(op->which_kind == obj->which_kind)) {

			if ((obj->what_is != WEAPON) ||
			((obj->what_is == WEAPON) &&
			((obj->which_kind == ARROW) ||
			(obj->which_kind == DAGGER) ||
			(obj->which_kind == DART) ||
			(obj->which_kind == SHURIKEN)) &&
			(obj->quiver == op->quiver))) {
				op->quantity += obj->quantity;
				return(op);
			}
		}
		op = op->next_object;
	}
	return(0);
}

next_avail_ichar()
{
	register object *obj;
	register i;
	boolean ichars[26];

	for (i = 0; i < 26; i++) {
		ichars[i] = 0;
	}
	obj = rogue.pack.next_object;
	while (obj) {
		ichars[(obj->ichar - 'a')] = 1;
		obj = obj->next_object;
	}
	for (i = 0; i < 26; i++) {
		if (!ichars[i]) {
			return(i + 'a');
		}
	}
	return('?');
}

wait_for_ack()
{
	while (rgetchar() != ' ') ;
}

pack_letter(prompt, mask)
char *prompt;
unsigned short mask;
{
	short ch;
	unsigned short tmask = mask;

	if (!mask_pack(&rogue.pack, mask)) {
		message(mesg[93], 0);
		return(CANCEL);
	}
	for (;;) {

		message(prompt, 0);

		for (;;) {
			ch = rgetchar();
			if (!is_pack_letter(&ch, &mask)) {
				sound_bell();
			} else {
				break;
			}
		}

		if (ch == LIST) {
			check_message();
			inventory(&rogue.pack, mask);
		} else {
			break;
		}
		mask = tmask;
	}
	check_message();
	return(ch);
}

take_off()
{
	char desc[DCOLS];
	object *obj;

	if (rogue.armor) {
		if (rogue.armor->is_cursed) {
			message(curse_message, 0);
		} else {
			mv_aquatars();
			obj = rogue.armor;
			unwear(rogue.armor);
#ifdef JAPAN
			get_desc(obj, desc, 0);
			(void) strcat(desc, mesg[94]);
#else
			(void) strcpy(desc, mesg[94]);
			get_desc(obj, desc+strlen(mesg[94]), 0);
#endif
			message(desc, 0);
			print_stats(STAT_ARMOR);
			(void) reg_move();
		}
	} else {
		message(mesg[95], 0);
	}
}

wear()
{
	short ch;
	register object *obj;
	char desc[DCOLS];

	if (rogue.armor) {
		message(mesg[96], 0);
		return;
	}
	ch = pack_letter(mesg[97], ARMOR);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(mesg[98], 0);
		return;
	}
	if (obj->what_is != ARMOR) {
		message(mesg[99], 0);
		return;
	}
	obj->identified = 1;
#ifdef JAPAN
	get_desc(obj, desc, 0);
	(void) strcat(desc, mesg[100]);
#else
	(void) strcpy(desc, mesg[100]);
	get_desc(obj, desc + strlen(mesg[100]), 0);
#endif
	message(desc, 0);
	do_wear(obj);
	print_stats(STAT_ARMOR);
	(void) reg_move();
}

unwear(obj)
object *obj;
{
	if (obj) {
		obj->in_use_flags &= (~BEING_WORN);
	}
	rogue.armor = (object *) 0;
}

do_wear(obj)
object *obj;
{
	rogue.armor = obj;
	obj->in_use_flags |= BEING_WORN;
	obj->identified = 1;
}

wield()
{
	short ch;
	register object *obj;
	char desc[DCOLS];

	if (rogue.weapon && rogue.weapon->is_cursed) {
		message(curse_message, 0);
		return;
	}
	ch = pack_letter(mesg[101], WEAPON);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(mesg[102], 0);
		return;
	}
	if (obj->what_is & (ARMOR | RING)) {
		sprintf(desc, mesg[103],
			((obj->what_is == ARMOR) ? mesg[104] : mesg[105]));
		message(desc, 0);
		return;
	}
	if (obj->in_use_flags & BEING_WIELDED) {
		message(mesg[106], 0);
	} else {
		unwield(rogue.weapon);
#ifdef JAPAN
		get_desc(obj, desc, 0);
		(void) strcat(desc, mesg[107]);
#else
		(void) strcpy(desc, mesg[107]);
		get_desc(obj, desc + strlen(mesg[107]), 0);
#endif
		message(desc, 0);
		do_wield(obj);
		(void) reg_move();
	}
}

do_wield(obj)
object *obj;
{
	rogue.weapon = obj;
	obj->in_use_flags |= BEING_WIELDED;
}

unwield(obj)
object *obj;
{
	if (obj) {
		obj->in_use_flags &= (~BEING_WIELDED);
	}
	rogue.weapon = (object *) 0;
}

call_it()
{
	short ch;
	register object *obj;
	struct id *id_table;
	char buf[MAX_TITLE_LENGTH+2];

	ch = pack_letter(mesg[108],
			(SCROL | POTION | WAND | RING));

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(mesg[109], 0);
		return;
	}
	if (!(obj->what_is & (SCROL | POTION | WAND | RING))) {
		message(mesg[110], 0);
		return;
	}
	id_table = get_id_table(obj);

#ifdef JAPAN
	if (get_input_line(mesg[111],
			"", buf, id_table[obj->which_kind].title, 0, 1)) {
		ch = *buf;
#ifdef EUC
		if (ch >= ' ' && !(ch & 0x80)) {	/* by Yasha */
#else	/* Shift JIS */
		if (ch >= ' ' && ch <= '~' || ch >= 0xa0 && ch <= 0xde) {
#endif
			/* alphabet or kana character; append 1 blank */
			(void) strcat(buf, " ");
		}
		id_table[obj->which_kind].id_status = CALLED;
		(void) strcpy(id_table[obj->which_kind].title, buf);
	}
#else
	if (get_input_line(mesg[111],
			"", buf, id_table[obj->which_kind].title, 1, 1)) {
		id_table[obj->which_kind].id_status = CALLED;
		(void) strcpy(id_table[obj->which_kind].title, buf);
	}
#endif
}

pack_count(new_obj)
object *new_obj;
{
	object *obj;
	short count = 0;

	obj = rogue.pack.next_object;

	while (obj) {
		if (obj->what_is != WEAPON) {
			count += obj->quantity;
		} else if (!new_obj) {
			count++;
		} else if ((new_obj->what_is != WEAPON) ||
			((obj->which_kind != ARROW) &&
			(obj->which_kind != DAGGER) &&
			(obj->which_kind != DART) &&
			(obj->which_kind != SHURIKEN)) ||
			(new_obj->which_kind != obj->which_kind) ||
			(obj->quiver != new_obj->quiver)) {
			count++;
		}
		obj = obj->next_object;
	}
	return(count);
}

boolean
mask_pack(pack, mask)
object *pack;
unsigned short mask;
{
	while (pack->next_object) {
		pack = pack->next_object;
		if (pack->what_is & mask) {
			return(1);
		}
	}
	return(0);
}

is_pack_letter(c, mask)
short *c;
unsigned short *mask;
{
	switch(*c) {
	case '?': *mask = SCROL;  goto found;
	case '!': *mask = POTION; goto found;
	case ':': *mask = FOOD;   goto found;
	case ')': *mask = WEAPON; goto found;
	case ']': *mask = ARMOR;  goto found;
	case '/': *mask = WAND;   goto found;
	case '=': *mask = RING;   goto found;
	case ',': *mask = AMULET; goto found;
	default: return(*c >= 'a' && *c <= 'z' || *c == CANCEL || *c == LIST);
	}
found:
	*c = LIST;
	return(1);
}

has_amulet()
{
	return(mask_pack(&rogue.pack, AMULET));
}

kick_into_pack()
{
	object *obj;
	char *p;
	char desc[DCOLS];
	short stat;
	extern short levitate;

	if (!(dungeon[rogue.row][rogue.col] & OBJECT)) {
		message(mesg[112], 0);
	} else {
#ifndef ORIGINAL
		if (levitate) {
			message(mesg[113], 0);
			return 0;
		}
#endif
		if (obj = pick_up(rogue.row, rogue.col, &stat)) {
			get_desc(obj, desc, 1);
#ifdef JAPAN
			(void) strcat(desc, mesg[114]);
#endif
			if (obj->what_is == GOLD) {
				message(desc, 0);
				free_object(obj);
			} else {
				p = desc + strlen(desc);
				*p++ = '(';
				*p++ = obj->ichar;
				*p++ = ')';
				*p = 0;
				message(desc, 0);
			}
		}
		if (obj || (!stat)) {
			(void) reg_move();
		}
	}
}
