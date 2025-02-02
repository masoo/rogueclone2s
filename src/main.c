/*
 * main.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utf8.h"

#include "init.h"
#include "level.h"
#include "main.h"
#include "message.h"
#include "monster.h"
#include "object.h"
#include "play.h"
#include "random.h"
#include "rogue.h"
#include "trap.h"

extern short party_room;
extern char *nick_name;
static char *progname;
utf8_int8_t mesg[MESSAGE_QUANTITY + 1]
		[MAX_MESG_BUFFER_SIZE]; /* for separation */
utf8_mesg umesg[MESSAGE_QUANTITY + 1];

int
main(int argc, char *argv[])
{
	int first = 1;
	char buf[80];

	progname = argv[0];

	if (init(argc, argv)) { /* restored game */
		goto PL;
	}

	for (;;) {
		clear_level();
		make_level();
		put_objects();
		put_stairs();
		add_traps();
		put_mons();
		put_player(party_room);
		print_stats(STAT_ALL);
		if (first) {
			sprintf(buf, mesg[10], nick_name);
			message(buf, 0);
		}
	PL:
		first = 0;
		play_level();
		free_stuff(&level_objects);
		free_stuff(&level_monsters);
	}
}

/*
 * read_mesg
 * メッセージファイル読み込み関数
 * 返り値: 成功:0, 失敗:1
 */
int
read_mesg(char *argv_msgfile)
{
	FILE *mesg_file;
	utf8_int8_t buf[MAX_MESG_LINE_SIZE];
	int e;

	if ((mesg_file = fopen(argv_msgfile, "r")) == NULL) {
		fprintf(stderr, "Cannot open message file '%s'\n",
		    argv_msgfile);
		return 1;
	}

	umesg[0].string[0] = '\0';
	umesg[0].size = 0;
	while (fgets(buf, sizeof(buf), mesg_file) != NULL) {
		int n = atoi(buf);
		int i, s;
		if (n > 0 && n <= MESSAGE_QUANTITY) {
			for (i = 0; buf[i] && buf[i] != '\"'; ++i) {
				continue;
			}
			if (buf[i]) {
				s = i + 1;
			} else {
			FMTERR:
				fprintf(stderr, "Illegal format '%s'\n",
				    argv_msgfile);
				return 1;
			}
			for (i = s; buf[i] && buf[i] != '\"'; ++i) {
				continue;
			}
			if (buf[i]) {
				e = i - 1;
			} else {
				goto FMTERR;
			}

			for (i = 0; i < e - s + 1 && i < MAX_MESG_LINE_SIZE - 1;
			    ++i) {
				mesg[n][i] = buf[s + i];
				umesg[n].string[i] = buf[s + i];
			}
			mesg[n][i] = '\0';
			umesg[n].string[i] = '\0';
			umesg[n].size = utf8size(umesg[n].string);
		}
	}
	return 0;
}

/*
 * usage
 * 使い方
 */
void
usage()
{
	fprintf(stderr, "usage: %s message_file [options...] [save_file]\n",
	    progname);
	exit(1);
}
