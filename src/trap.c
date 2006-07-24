/*
 * trap.c
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
#include <stdbool.h>
#include <curses.h>

#include "rogue.h"
#include "trap.h"
#include "display.h"
#include "hit.h"
#include "message.h"
#include "move.h"
#include "object.h"
#include "random.h"
#include "room.h"
#include "save.h"
#include "score.h"
#include "spechit.h"
#include "use.h"

trap traps[MAX_TRAPS];
bool trap_door = false;
short bear_trap = 0;

char *trap_strings[TRAPS * 2] = {
    mesg[216], mesg[217], mesg[218], mesg[219], mesg[220], mesg[221],
    mesg[222], mesg[223], mesg[224], mesg[225], mesg[226], mesg[227]
};

extern short cur_level, party_room;
extern char *new_level_message;
extern bool interrupted;
extern short ring_exp;
extern bool sustain_strength;
extern short blind;

int
trap_at(int row, int col)
{
    short i;

    for (i = 0; ((i < MAX_TRAPS) && (traps[i].trap_type != NO_TRAP)); i++) {
	if ((traps[i].trap_row == row) && (traps[i].trap_col == col)) {
	    return (traps[i].trap_type);
	}
    }
    return (NO_TRAP);
}

void
trap_player(short row, short col)
{
    short t;

    if ((t = trap_at(row, col)) == NO_TRAP) {
	return;
    }
    dungeon[row][col] &= (~HIDDEN);
    if (rand_percent(rogue.exp + ring_exp)) {
	message(mesg[228], 1);
	return;
    }
    switch (t) {
    case TRAP_DOOR:
	trap_door = true;
	new_level_message = trap_strings[(t * 2) + 1];
	break;
    case BEAR_TRAP:
	message(trap_strings[(t * 2) + 1], 1);
	bear_trap = get_rand(4, 7);
	break;
    case TELE_TRAP:
	mvaddch_rogue(rogue.row, rogue.col, '^');
	tele();
	break;
    case DART_TRAP:
	message(trap_strings[(t * 2) + 1], 1);
	rogue.hp_current -= get_damage("1d6", 1);
	if (rogue.hp_current <= 0) {
	    rogue.hp_current = 0;
	}
	if ((!sustain_strength) && rand_percent(40) &&
	    (rogue.str_current >= 3)) {
	    rogue.str_current--;
	}
	print_stats(STAT_HP | STAT_STRENGTH);
	if (rogue.hp_current <= 0) {
	    killed_by((object *) 0, POISON_DART);
	}
	break;
    case SLEEPING_GAS_TRAP:
	message(trap_strings[(t * 2) + 1], 1);
	take_a_nap();
	break;
    case RUST_TRAP:
	message(trap_strings[(t * 2) + 1], 1);
	rust((object *) 0);
	break;
    }
}

void
add_traps(void)
{
    short i, n, tries = 0;
    short row, col;

    if (cur_level <= 2) {
	n = 0;
    } else if (cur_level <= 7) {
	n = get_rand(0, 2);
    } else if (cur_level <= 11) {
	n = get_rand(1, 2);
    } else if (cur_level <= 16) {
	n = get_rand(2, 3);
    } else if (cur_level <= 21) {
	n = get_rand(2, 4);
    } else if (cur_level <= (AMULET_LEVEL + 2)) {
	n = get_rand(3, 5);
    } else {
	n = get_rand(5, MAX_TRAPS);
    }
    for (i = 0; i < n; i++) {
	traps[i].trap_type = get_rand(0, (TRAPS - 1));

	if ((i == 0) && (party_room != NO_ROOM)) {
	    do {
		row = get_rand((rooms[party_room].top_row + 1),
			       (rooms[party_room].bottom_row - 1));
		col = get_rand((rooms[party_room].left_col + 1),
			       (rooms[party_room].right_col - 1));
		tries++;
	    } while (((dungeon[row][col] & (OBJECT | STAIRS | TRAP | TUNNEL))
		      || (dungeon[row][col] == NOTHING)) && (tries < 15));
	    if (tries >= 15) {
		gr_row_col(&row, &col, (FLOOR | MONSTER));
	    }
	} else {
	    gr_row_col(&row, &col, (FLOOR | MONSTER));
	}
	traps[i].trap_row = row;
	traps[i].trap_col = col;
	dungeon[row][col] |= (TRAP | HIDDEN);
    }
}

void
id_trap(void)
{
    short dir, row, col;
    short t;

    dir = get_direction();
    if (dir == CANCEL) {
	return;
    }
    row = rogue.row;
    col = rogue.col;

    get_dir_rc(dir, &row, &col, false);

    if ((dungeon[row][col] & TRAP) && (!(dungeon[row][col] & HIDDEN))) {
	t = trap_at(row, col);
	message(trap_strings[t * 2], 0);
    } else {
	message(mesg[229], 0);
    }
}

void
show_traps(void)
{
    short i, j;

    for (i = 0; i < ROGUE_LINES; i++) {
	for (j = 0; j < ROGUE_COLUMNS; j++) {
	    if (dungeon[i][j] & TRAP) {
		mvaddch_rogue(i, j, '^');
	    }
	}
    }
}

void
search(short n, bool is_auto)
{
    short s, i, j, row, col, t;
    short shown = 0, found = 0;
    static bool reg_search;

    for (i = -1; i <= 1; i++) {
	for (j = -1; j <= 1; j++) {
	    row = rogue.row + i;
	    col = rogue.col + j;
	    if ((row < MIN_ROW) || (row >= (ROGUE_LINES - 1)) ||
		(col < 0) || (col >= ROGUE_COLUMNS)) {
		continue;
	    }
	    if (dungeon[row][col] & HIDDEN) {
		found++;
	    }
	}
    }
    for (s = 0; s < n; s++) {
	for (i = -1; i <= 1; i++) {
	    for (j = -1; j <= 1; j++) {
		row = rogue.row + i;
		col = rogue.col + j;
		if ((row < MIN_ROW) || (row >= (ROGUE_LINES - 1)) ||
		    (col < 0) || (col >= ROGUE_COLUMNS)) {
		    continue;
		}
		if (dungeon[row][col] & HIDDEN) {
		    if (rand_percent(17 + (rogue.exp + ring_exp))) {
			dungeon[row][col] &= (~HIDDEN);
			if ((!blind) && ((row != rogue.row) ||
					 (col != rogue.col))) {
			    mvaddch_rogue(row, col,
					  get_dungeon_char(row, col));
			}
			shown++;
			if (dungeon[row][col] & TRAP) {
			    t = trap_at(row, col);
			    message(trap_strings[t * 2], 1);
			}
		    }
		}
		if (((shown == found) && (found > 0)) || interrupted) {
		    return;
		}
	    }
	}
	if ((!is_auto) && (reg_search = !reg_search)) {
	    (void) reg_move();
	}
    }
}
