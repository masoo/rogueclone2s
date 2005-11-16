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

#include <stdio.h>
#include "rogue.h"

extern short party_room;
#ifndef ORIGINAL
extern char *nick_name;
char mesg[507][80];	/* for separation */
#endif

main(argc, argv)
int argc;
char *argv[];
{
#ifndef ORIGINAL
	int first = 1;
	char buf[80];
#endif

	if (read_mesg(argc, argv))
		exit(1);

	if (init(argc - 1, argv + 1)) {		/* restored game */
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
#ifndef ORIGINAL
		if (first) {
			sprintf(buf,
				mesg[10],
				nick_name);
			message(buf, 0);
		}
	PL:
		first = 0;
#else
	PL:
#endif
		play_level();
		free_stuff(&level_objects);
		free_stuff(&level_monsters);
	}
}

int read_mesg(ac, av)
int ac;
char **av;
{
	FILE *mesg_file;
	char buf[256];
	int i, n, s, e;

	if (ac < 2) {
		fprintf(stderr, "%s: message_file [options...]\n", av[0]);
		return 1;
	}

	if ((mesg_file = fopen(av[1], "r")) == NULL) {
		fprintf(stderr, "Cannot open message file '%s'\n", av[1]);
		return 1;
	}

	while (fgets(buf, 256, mesg_file) != NULL) {
		if ((n = atoi(buf)) > 0 && n < 500) {
			for (i = 0; buf[i] && buf[i] != '\"'; ++i)
				;
			if (buf[i])
				s = i + 1;
			else {
FMTERR:				fprintf(stderr, "Illegal format '%s'\n",
					av[1]);
				return 1;
			}
			for (i = s; buf[i] && buf[i] != '\"'; ++i)
				;
			if (buf[i])
				e = i - 1;
			else
				goto FMTERR;

			for (i = 0; i < e-s+1 && i < 79; ++i)
				mesg[n][i] = buf[s + i];
			mesg[n][i] = '\0';
		}
	}
	return 0;
}
