/*
 * init.c
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

char login_name[30];
char *nick_name = "";
char *rest_file = 0;
boolean cant_int = 0, did_int = 0, score_only = 0, init_curses = 0;
boolean save_is_interactive = 1;
boolean show_skull = 1;
#ifndef MSDOS
boolean ask_quit = 1;
#endif
#ifndef ORIGINAL
boolean pass_go = 1, do_restore = 0;
char org_dir[64], *game_dir = "";
#endif
#ifdef COLOR
char *color_str = "cbmyg";
boolean do_color = 1;
short c_buf[5];
short c_attr[256];
#endif
#ifdef UNIX
char *error_file = "rogue.esave";
#endif
#ifdef MSDOS
char *error_file = "rogue.err";
char *mac_type = "none", *cursor_str = "";
char *init_string = "", *term_string = "";
char init_str[30], term_str[30];
char cursor_on[10], cursor_off[10];
#endif

extern char *fruit;
extern char *save_file;
extern short party_room, party_counter;
extern boolean jump;

init(argc, argv)
int argc;
char *argv[];
{
	char *pn;
	int seed;

#ifndef ORIGINAL
	md_getcwd(org_dir, 64);
#endif
	do_args(argc, argv);
#ifdef COLOR
	init_color();
#endif
	do_opts();

	pn = md_gln();
	if ((!pn) || (strlen(pn) >= 30)) {
		clean_up(mesg[13]);
	}
	(void) strcpy(login_name, pn);
	if (!nick_name[0])
		nick_name = login_name;

	initscr();

#ifndef MSDOS
	if ((LINES < DROWS) || (COLS < DCOLS)) {
		clean_up(mesg[14]);
	}
#endif

	start_window();
	init_curses = 1;
	md_heed_signals();

	if (score_only) {
		message("", 0);		/* by Yasha */
		put_scores((object *) 0, 0);
	}
	seed = md_gseed();
	(void) srrandom(seed);
#ifndef ORIGINAL
	if (do_restore && save_file && *save_file)
		rest_file = save_file;
#endif
	if (rest_file) {
		restore(rest_file);
		return(1);
	}
	mix_colors();
	get_wand_and_ring_materials();
	make_scroll_titles();

	level_objects.next_object = 0;
	level_monsters.next_monster = 0;
	player_init();
	party_counter = get_rand(1, PARTY_TIME);
	ring_stats(0);
	return(0);
}

player_init()
{
	object *obj;

	rogue.pack.next_object = 0;

	obj = alloc_object();
	get_food(obj, 1);
	(void) add_to_pack(obj, &rogue.pack, 1);

	obj = alloc_object();		/* initial armor */
	obj->what_is = ARMOR;
	obj->which_kind = RINGMAIL;
	obj->class = RINGMAIL+2;
	obj->is_protected = 0;
	obj->d_enchant = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);
	do_wear(obj);

	obj = alloc_object();		/* initial weapons */
	obj->what_is = WEAPON;
	obj->which_kind = MACE;
	obj->damage = "2d3";
	obj->hit_enchant = obj->d_enchant = 1;
	obj->identified = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);
	do_wield(obj);

	obj = alloc_object();
	obj->what_is = WEAPON;
	obj->which_kind = BOW;
	obj->damage = "1d2";
	obj->hit_enchant = 1;
	obj->d_enchant = 0;
	obj->identified = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);

	obj = alloc_object();
	obj->what_is = WEAPON;
	obj->which_kind = ARROW;
	obj->quantity = get_rand(25, 35);
	obj->damage = "1d2";
	obj->hit_enchant = 0;
	obj->d_enchant = 0;
	obj->identified = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);
}

clean_up(estr)
char *estr;
{
	if (save_is_interactive) {
		if (init_curses) {
			move(DROWS-1, 0);
#ifndef ORIGINAL
			clrtoeol();
#endif
#ifdef MSDOS
			move(DROWS-4, 0);
#endif
			refresh();
			stop_window();
		}
#if defined(JAPAN) && !defined(CURSES)
		raw();			/* by Yasha */
#endif
		putstr("\r\n");
		putstr(estr);
		putstr("\r\n");
#if defined(JAPAN) && !defined(CURSES)
		md_control_keyboard(1);		/* by Yasha */
#endif
	}
	md_exit(0);
}

start_window()
{
#ifdef MSDOS
	if (*init_str)
		putstr(init_str);
	md_cbreak_no_echo_nonl(1);
#else
	crmode();
	noecho();
#if defined(JAPAN) && !defined(CURSES)
	raw();		/* by Yasha */
#endif
#ifndef BAD_NONL
	nonl();
#endif /*BAD_NONL*/
	md_control_keyboard(0);
#endif /*MSDOS*/
}

stop_window()
{
	endwin();
#ifndef MSDOS
	md_control_keyboard(1);
#endif
#ifdef MSDOS
	if (*term_str)
		putstr(term_str);
#endif
}

#ifndef MSDOS
byebye()
{
	md_ignore_signals();
	if (ask_quit) {
		quit(1);
	} else {
		clean_up(mesg[12]);	/* byebye_string */
	}
	md_heed_signals();
}
#endif /*MSDOS*/

#if defined(MSDOS) && (!defined(__TURBOC__) || __TURBOC__ >= 0x0200)
void
#endif
onintr()
{
	md_ignore_signals();
	if (cant_int) {
		did_int = 1;
	} else {
		check_message();
		message(mesg[15], 1);
	}
	md_heed_signals();
#if defined(MSDOS) && defined(__TURBOC__) && __TURBOC__ < 0x0200
	return 1;
#endif
}

#if defined(MSDOS) && defined(__TURBOC__) && __TURBOC__ < 0x0200
ignintr()
{
	return 1;
}
#endif

error_save()
{
	save_is_interactive = 0;
	save_into_file(error_file);
	clean_up("");
}

do_args(argc, argv)
int argc;
char *argv[];
{
	short i, j;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			for (j = 1; argv[i][j]; j++) {
				switch(argv[i][j]) {
				case 's':
					score_only = 1;
					break;
#ifndef ORIGINAL
				case 'r':
					do_restore = 1;
					break;
#endif
				}
			}
		} else {
			rest_file = argv[i];
		}
	}
}

opt envopt[] = {
#ifndef MSDOS
	{ "askquit",	&ask_quit,	NULL,		0, 0 },
#endif
	{ "jump",	&jump,		NULL,		0, 0 },
#ifndef ORIGINAL
	{ "passgo",	&pass_go,	NULL,		0, 0 },
	{ "tombstone",	&show_skull,	NULL,		0, 0 },
#else
	{ "skull",	&show_skull,	NULL,		0, 0 },
#endif
#ifdef COLOR
	{ "color",	&do_color,	NULL,		0, 0 },
#endif
#ifdef JAPAN
	{ "fruit",	NULL,		&fruit,		0, 0 },
#else
	{ "fruit",	NULL,		&fruit,		1, 0 },
#endif
	{ "file",	NULL,		&save_file,	0, 0 },
	{ "name",	NULL,		&nick_name,	0, 1 },
#ifndef ORIGINAL
	{ "directory",	NULL,		&game_dir,	0, 0 },
#endif
#ifdef COLOR
	{ "map",	NULL,		&color_str,	0, 0 },
#endif
#ifdef MSDOS
	{ "type",	NULL,		&mac_type,	0, 0 },
	{ "init",	NULL,		&init_string,	0, 0 },
	{ "term",	NULL,		&term_string,	0, 0 },
	{ "cursor",	NULL,		&cursor_str,	0, 0 },
#endif
	{ NULL,		NULL,		NULL,		0, 0 }
};

#ifdef JAPAN
char *optdesc[] = {
#ifndef MSDOS
	"��λ���뤫�ɤ�����ǧ��Ȥ�",
#endif
	"��ư���ɽ����Ԥ�ʤ�",
#ifndef ORIGINAL
	"��ϩ�γѤǻߤޤ餺�˿ʤ�",
	"�����ཪλ������ɸ��ɽ������",
#else
	"�����ཪλ���˳�����ɽ������",
#endif
#ifdef COLOR
	"����饯�����򥫥顼��ɽ������",
#endif
	mesg[16],
	"�����֥ե�����̾",
	"�˥å��͡���",
#ifndef ORIGINAL
	"������ǥ��쥯�ȥ꡼̾",
#endif
#ifdef COLOR
	"����饯������ɽ�����ޥåԥ�",
#endif
#ifdef MSDOS
	"���ѵ���",
	"�������Ѥ򳫻Ϥ��륷������",
	"�������Ѥ�λ���륷������",
	"�������륪�󡿥��դΥ�������",
#endif
	NULL
};
#else /*JAPAN*/
char *optdesc[] = {
#ifndef MSDOS
	"Ask whether quit or not on quit signal",
#endif
	"Show position only at end of run",
#ifndef ORIGINAL
	"Follow turnings in passageways",
	"Print out tombstone and score when killed",
#else
	"Print out skull and score when killed",
#endif
#ifdef COLOR
	"Show characters in map with color",
#endif
	mesg[16],
	"Save filename",
	"Your nickname",
#ifndef ORIGINAL
	"Game directory name",
#endif
#ifdef COLOR
	"Color mapping for characters",
#endif
#ifdef MSDOS
	"Machine type",
	"Initialize sequence in hex",
	"Terminate sequence in hex",
	"Cursor on/off sequence in hex/hex",
#endif
	NULL
};
#endif /*JAPAN*/

#ifdef MSDOS
typedef struct machine { char *type, *cursor, *init, *term;} mac;
mac macs[] = {
{"pc98",  "1b5b3e356c/1b5b3e3568", "1b29301b5b3e336c1b5b3e3168", "1b5b3e316c"},
{"pc100", "1b5b3e3568/1b5b3e356c", "1b29301b5b3d33681b5b3e316c", "1b5b3e3168"},
{"ax",    "1b5b3e356c/1b5b3e3568", "", ""},
{"fmr",   "1b5b3076/1b5b3176", "", ""},
{"b16",   "1b5b3e356c/1b5b3e3568", "", ""},
{"if800", "1b31/1b30", "1b4e301b6e391b53202030", "1b53202031"},
{NULL, NULL, NULL, NULL}
};
#endif /*MSDOS*/

do_opts()
{
	register char *ep, *p;
	char envname[10];
	char envbuf[BUFSIZ];

	strcpy(envname, "ROGUEOPT?");
	envbuf[0] = 0;
	for (p = "S123456789"; *p; p++) {
		envname[8] = *p;
		if (ep = md_getenv(envname)) {
			strcat(envbuf, ",");
			strcat(envbuf, ep);
		}
	}
	set_opts(envbuf);
}

set_opts(env)
char *env;
{
	short not, i;
	register char *ep, *p;
	opt *op;
#ifdef MSDOS
	mac *mp;
#endif
	char optname[20];

	if (*env == 0)
		return;
	ep = env;
	for (;;) {
		while (*ep == ' ' || *ep == ',')
			ep++;

		if (*ep == 0)
			break;

		not = 0;
		if (!strncmp("no", ep, 2) || !strncmp("NO", ep, 2)) {
			not = 1;
			ep += 2;
		}
		p = optname;
		while (*ep && *ep != ',' && *ep != '=' && *ep != ':')
			*p++ = (*ep>='A'&&*ep<='Z')? (*ep++)-'A'+'a': *ep++;
		*p = 0;
		for (op = envopt; op->name; op++) {
			if (strncmp(op->name, optname, strlen(optname)))
				continue;
			if (op->bp)
				*(op->bp) = !not;
			if (op->cp && (*ep == '=' || *ep == ':'))
				env_get_value(op->cp, ep+1, op->ab, op->nc);
		}

		while (*ep && *ep != ',')
			ep++;
	}

#ifdef MSDOS
	for (p = mac_type; *p; p++)
		if (*p >= 'A' && *p <= 'Z')
			*p += 'a' - 'A';
	for (mp = macs; mp->type; mp++) {
		if (!strcmp(mac_type, mp->type)) {
			cursor_str  = mp->cursor;
			init_string = mp->init;
			term_string = mp->term;
			break;
		}
	}
	if (init_string) {
		p = init_string;
		ep = init_str;
		while ((i = get_hex_num(p, 2)) >= 0) {
			*ep++ = i;
			p += 2;
		}
		*ep = 0;
	}
	if (term_string) {
		p = term_string;
		ep = term_str;
		while ((i = get_hex_num(p, 2)) >= 0) {
			*ep++ = i;
			p += 2;
		}
		*ep = 0;
	}
	if (cursor_str) {
		p = cursor_str;
		ep = cursor_on;
		while ((i = get_hex_num(p, 2)) >= 0) {
			*ep++ = i;
			p += 2;
		}
		*ep = 0;
		if (*p)
			p++;
		ep = cursor_off;
		while ((i = get_hex_num(p, 2)) >= 0) {
			*ep++ = i;
			p += 2;
		}
		*ep = 0;
	}
#endif

#ifdef COLOR
	init_color();
#endif

#ifndef ORIGINAL
	if (game_dir && *game_dir)
		md_chdir(game_dir);
#endif

#ifdef COLOR
	if (init_curses)
		repaint_screen();
#endif
}

#ifdef COLOR
init_color()
{
	register short i, j;
	char *p;

	if (color_str && *color_str) {
		for (i = 0; i < 5 && color_str[i]; i++) {
			j = r_index("wrgybmc?WRGYBMC", color_str[i], 0);
			if (j >= 0)
				c_buf[i] = j;
		}
		for (p = "-|#+"; *p; p++)
			c_attr[*p] = c_buf[0] << 8;
		c_attr['.'] = c_buf[1] << 8;
		for (i = 'A'; i <= 'Z'; i++)
			c_attr[i] = c_buf[2] << 8;
		for (p = "%!?/=)]^*:,"; *p; p++)
			c_attr[*p] = c_buf[3] << 8;
		c_attr[rogue.fchar] = c_buf[4] << 8;
	} else {
		for (i = 0; i < 5; i++)
			c_buf[i] = 0;
		for (i = 0; i < 256; i++)
			c_attr[i] = 0;
	}
}
#endif

#ifdef MSDOS
get_hex_num(p, n)
register char *p;
int n;
{
	int x;

	x = 0;
	while (n-- > 0) {
		x <<= 4;
		if (*p >= '0' && *p <= '9')
			x += *p++ - '0';
		else if (*p >= 'a' && *p <= 'z')
			x += *p++ - 'a' + 10;
		else if (*p >= 'A' && *p <= 'Z')
			x += *p++ - 'A' + 10;
		else
			return -1;
	}
	return x;
}
#endif /* MSDOS */

env_get_value(s, e, add_blank, no_colon)
char **s, *e;
boolean add_blank, no_colon;
{
	short i = 0;
	char *t;

	t = e;

	while ((*e) && (*e != ',')) {
#ifndef ORIGINAL
#ifdef EUC
		if (*e & 0x80) {	/* by Yasha */
#else
		if (*e >'\200' && *e <'\240' || *e >= '\340' && *e < '\360') {
#endif
			e += 2;
			i += 2;
			continue;
		}
#endif
		if (*e == ':' && no_colon) {
			*e = ';'; /* ':' reserved for score file purposes */
		}
		e++;
		if (++i >= 30) {
			break;
		}
	}
	if (!(*s = md_malloc(i + (add_blank ? 2 : 1)))) {
		clean_up(mesg[17]);
	}
	(void) strncpy(*s, t, i);
	if (add_blank) {
		(*s)[i++] = ' ';
	}
	(*s)[i] = '\0';
}
