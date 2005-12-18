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
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>

#include "rogue.h"
#include "init.h"
#include "display.h"
#include "invent.h"
#include "machdep.h"
#include "main.h"
#include "message.h"
#include "object.h"
#include "pack.h"
#include "random.h"
#include "ring.h"
#include "save.h"
#include "score.h"

char login_name[30];
char *nick_name = "";
char *rest_file = 0;
boolean cant_int = 0, did_int = 0, score_only = 0, init_curses = 0;
boolean save_is_interactive = 1;
boolean show_skull = 1;
boolean ask_quit = 1;
#ifndef ORIGINAL
boolean pass_go = 1, do_restore = 0;
char org_dir[64], *game_dir = "";
#endif
boolean use_color = 1;
char *error_file = "rogue.esave";

extern char *fruit;
extern char *save_file;
extern short party_room, party_counter;
extern boolean jump;

int
init(int argc, char *argv[])
{
    char *pn;
    int seed;
    WINDOW *main_window;

#ifndef ORIGINAL
    md_getcwd(org_dir, 64);
#endif
    do_args(argc, argv);
    do_opts();

    pn = md_gln();
    if ((!pn) || (strlen(pn) >= 30)) {
	clean_up(mesg[13]);
    }
    (void) strcpy(login_name, pn);
    if (!nick_name[0]) {
	nick_name = login_name;
    }

    /* init curses */
    main_window = initscr();
    if (main_window == NULL) {
	return 1;
    }
    init_color_attr();

    if ((LINES < ROGUE_LINES) || (COLS < ROGUE_COLUMNS)) {
	clean_up(mesg[14]);
    }

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
    if (do_restore && save_file && *save_file) {
	rest_file = save_file;
    }
#endif
    if (rest_file) {
	restore(rest_file);
	return (1);
    }
    mix_colors();
    get_wand_and_ring_materials();
    make_scroll_titles();

    level_objects.next_object = 0;
    level_monsters.next_monster = 0;
    player_init();
    party_counter = get_rand(1, PARTY_TIME);
    ring_stats(0);
    return 0;
}

void
player_init(void)
{
    object *obj;

    rogue.pack.next_object = 0;

    obj = alloc_object();
    get_food(obj, 1);
    (void) add_to_pack(obj, &rogue.pack, 1);

    obj = alloc_object();	/* initial armor */
    obj->what_is = ARMOR;
    obj->which_kind = RINGMAIL;
    obj->class = RINGMAIL + 2;
    obj->is_protected = 0;
    obj->d_enchant = 1;
    (void) add_to_pack(obj, &rogue.pack, 1);
    do_wear(obj);

    obj = alloc_object();	/* initial weapons */
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

void
clean_up(char *estr)
{
    if (save_is_interactive) {
	if (init_curses) {
	    move(ROGUE_LINES - 1, 0);
#ifndef ORIGINAL
	    clrtoeol();
#endif
	    refresh();
	    stop_window();
	}
	endwin();
	printf("\r\n");
	printf(estr);
	printf("\r\n");
    }
    md_exit(0);
}

void
start_window(void)
{
    crmode();
    noecho();
#ifndef BAD_NONL
    nonl();
#endif /*BAD_NONL */
}

void
stop_window(void)
{
    endwin();
}

void
byebye(int sig)
{
    md_ignore_signals();
    if (ask_quit) {
	quit(1);
    } else {
	clean_up(mesg[12]);	/* byebye_string */
    }
    md_heed_signals();
}

void
onintr(int sig)
{
    md_ignore_signals();
    if (cant_int) {
	did_int = 1;
    } else {
	check_message();
	message(mesg[15], 1);
    }
    md_heed_signals();
}

void
error_save(int sig)
{
    save_is_interactive = 0;
    save_into_file(error_file);
    clean_up("");
}

void
do_args(int argc, char *argv[])
{
    int ch;
    char *option_strings;
    extern int optind;

#ifndef ORIGINAL
    option_strings = "sr";
#else
    option_strings = "s";
#endif

    while ((ch = getopt(argc, argv, option_strings)) != EOF) {
	switch (ch) {
	case 's':
            score_only = 1;
            break;
#ifndef ORIGINAL
        case 'r':
            do_restore = 1;
            break;
#endif
        case '?':
        default:
            usage();
            break;
        }
    }

    argc -= optind;
    argv += optind;
    if (argc >= 3 || argc == 0 ) {
        usage();
        return;
    }

    if (read_mesg(argv[0])) {
        exit(1);
    }

#ifndef ORIGINAL
    if (argc == 2) {
        rest_file = argv[1];
    }
#endif

}

opt envopt[] = {
    {"askquit", &ask_quit, NULL, 0, 0}
    ,
    {"jump", &jump, NULL, 0, 0}
    ,
#ifndef ORIGINAL
    {"passgo", &pass_go, NULL, 0, 0}
    ,
    {"tombstone", &show_skull, NULL, 0, 0}
    ,
#else
    {"skull", &show_skull, NULL, 0, 0}
    ,
#endif
#ifdef COLOR
    {"color", &use_color, NULL, 0, 0}
    ,
#endif
#ifdef JAPAN
    {"fruit", NULL, &fruit, 0, 0}
    ,
#else
    {"fruit", NULL, &fruit, 1, 0}
    ,
#endif
    {"file", NULL, &save_file, 0, 0}
    ,
    {"name", NULL, &nick_name, 0, 1}
    ,
#ifndef ORIGINAL
    {"directory", NULL, &game_dir, 0, 0}
    ,
#endif
#ifdef COLOR
    {"map", NULL, &color_str, 0, 0}
    ,
#endif
    {NULL, NULL, NULL, 0, 0}
};

#ifdef JAPAN
char *optdesc[] = {
    "終了するかどうか確認をとる",
    "移動中の表示を行わない",
#ifndef ORIGINAL
    "通路の角で止まらずに進む",
    "ゲーム終了時に墓標を表示する",
#else
    "ゲーム終了時に骸骨を表示する",
#endif
#ifdef COLOR
    "キャラクターをカラーで表示する",
#endif
    mesg[16],
    "セーブファイル名",
    "ニックネーム",
#ifndef ORIGINAL
    "ゲームディレクトリー名",
#endif
#ifdef COLOR
    "キャラクターの表示色マッピング",
#endif
    NULL
};
#else /*JAPAN*/
char *optdesc[] = {
    "Ask whether quit or not on quit signal",
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
    NULL
};
#endif /*JAPAN*/
    void
do_opts(void)
{
    char *ep, *p;
    char envname[10];
    char envbuf[BUFSIZ];

    strcpy(envname, "ROGUEOPT?");
    envbuf[0] = 0;
    for (p = "S123456789"; *p; p++) {
	envname[8] = *p;
	if ((ep = md_getenv(envname))) {
	    strcat(envbuf, ",");
	    strcat(envbuf, ep);
	}
    }
    set_opts(envbuf);
}

void
set_opts(char *env)
{
    short not;
    char *ep, *p;
    opt *op;
    char optname[20];

    if (*env == 0) {
	return;
    }
    ep = env;
    for (;;) {
	while (*ep == ' ' || *ep == ',') {
	    ep++;
	}

	if (*ep == 0) {
	    break;
	}

	not = 0;
	if (!strncmp("no", ep, 2) || !strncmp("NO", ep, 2)) {
	    not = 1;
	    ep += 2;
	}
	p = optname;
	while (*ep && *ep != ',' && *ep != '=' && *ep != ':') {
	    *p++ = (*ep >= 'A' && *ep <= 'Z') ? (*ep++) - 'A' + 'a' : *ep++;
	}
	*p = 0;
	for (op = envopt; op->name; op++) {
	    if (strncmp(op->name, optname, strlen(optname))) {
		continue;
	    }
	    if (op->bp) {
		*(op->bp) = !not;
	    }
	    if (op->cp && (*ep == '=' || *ep == ':')) {
		env_get_value(op->cp, ep + 1, op->ab, op->nc);
	    }
	}

	while (*ep && *ep != ',') {
	    ep++;
	}
    }

    init_color_attr();

#ifndef ORIGINAL
    if (game_dir && *game_dir) {
	md_chdir(game_dir);
    }
#endif
}

void
env_get_value(char **s, char *e, boolean add_blank, boolean no_colon)
{
    short i = 0;
    char *t;

    t = e;

    while ((*e) && (*e != ',')) {
#ifndef ORIGINAL
#ifdef EUC
	if (*e & 0x80) {	/* by Yasha */
#else
	if (*e > '\200' && *e < '\240' || *e >= '\340' && *e < '\360') {
#endif
	    e += 2;
	    i += 2;
	    continue;
	}
#endif
	if (*e == ':' && no_colon) {
	    *e = ';';		/* ':' reserved for score file purposes */
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
