/*
 * display.c
 * ɽ���ϥ�åѡ��ؿ����������
 */

#include <string.h>
#include <stdbool.h>
#include <curses.h>

#include "rogue.h"
#include "display.h"
#include "machdep.h"
#include "message.h"

/*
 * Ncurses Colors
 */
enum rogue_colors
{
    WHITE = 1,
    RED = 2,
    GREEN = 3,
    YELLOW = 4,
    BLUE = 5,
    MAGENTA = 6,
    CYAN = 7,
    WHITE_REVERSE = 8,
    RED_REVERSE = 9,
    GREEN_REVERSE = 10,
    YELLOW_REVERSE = 11,
    BLUE_REVERSE = 12,
    MAGENTA_REVERSE = 13,
    CYAN_REVERSE = 14
};
#define RWHITE	 8
#define RRED	 9
#define RGREEN	 10
#define RYELLOW	 11
#define RBLUE	 12
#define RMAGENTA 13
#define RCYAN	 14
static int ch_attr[256];
char *color_str = "cbmyg";
extern bool use_color;

/*
 * init_color_attr
 * ���顼°������ν����
 */
void
init_color_attr(void)
{
    char *chx, chy, *chz;
    int i, j, k;
    char color_type[] = "wrgybmcWRGYBMC";
    int colormap_list[5];

#if defined( COLOR )
    static bool first_init = true;

    /* �ǽ�ΰ��Τ߼¹Ԥ���̿�� */
    if (first_init) {
	start_color();

	/* �طʿ������ */
	assume_default_colors(COLOR_WHITE, COLOR_BLACK);

	/* ɽ��������� */
	init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(RED, COLOR_RED, COLOR_BLACK);
	init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(WHITE_REVERSE, COLOR_BLACK, COLOR_WHITE);
	init_pair(RED_REVERSE, COLOR_BLACK, COLOR_RED);
	init_pair(GREEN_REVERSE, COLOR_BLACK, COLOR_GREEN);
	init_pair(YELLOW_REVERSE, COLOR_BLACK, COLOR_YELLOW);
	init_pair(BLUE_REVERSE, COLOR_BLACK, COLOR_BLUE);
	init_pair(MAGENTA_REVERSE, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(CYAN_REVERSE, COLOR_BLACK, COLOR_CYAN);

	/* �ե饰�� OFF �ˤ��� */
	first_init = false;
    }
#endif /* COLOR */

    /* ɽ��������� */
    for (i = 0; i < 5 && color_str[i]; i++) {
	j = r_index(color_type, color_str[i], 0);
	if (j >= 0) {
	    switch (color_type[j]) {
	    default:
		colormap_list[i] = 0;
		break;
	    case 'w':
		colormap_list[i] = WHITE;
		break;
	    case 'r':
		colormap_list[i] = RED;
		break;
	    case 'g':
		colormap_list[i] = GREEN;
		break;
	    case 'y':
		colormap_list[i] = YELLOW;
		break;
	    case 'b':
		colormap_list[i] = BLUE;
		break;
	    case 'm':
		colormap_list[i] = MAGENTA;
		break;
	    case 'c':
		colormap_list[i] = CYAN;
		break;
	    case 'W':
		colormap_list[i] = WHITE_REVERSE;
		break;
	    case 'R':
		colormap_list[i] = RED_REVERSE;
		break;
	    case 'G':
		colormap_list[i] = GREEN_REVERSE;
		break;
	    case 'Y':
		colormap_list[i] = YELLOW_REVERSE;
		break;
	    case 'B':
		colormap_list[i] = BLUE_REVERSE;
		break;
	    case 'M':
		colormap_list[i] = MAGENTA_REVERSE;
		break;
	    case 'C':
		colormap_list[i] = CYAN_REVERSE;
		break;
	    }
	}
    }

    /* ʸ���Υ��顼�ޥåפκ��� */
    for (chx = "-|#+"; *chx; chx++) {
	get_colorpair_number((signed) *chx, colormap_list[0]);
    }
    get_colorpair_number((signed) '.', colormap_list[1]);
    for (chy = 'A'; chy <= 'Z'; chy++) {
	get_colorpair_number((signed) chy, colormap_list[2]);
    }
    for (chz = "%!?/=)]^*:,"; *chz; chz++) {
	get_colorpair_number((signed) *chz, colormap_list[3]);
    }
    get_colorpair_number(rogue.fchar, colormap_list[4]);

    if (!use_color) {
	for(k=0; k<128; k++) {
	    get_colorpair_number(k,0);
	}
    }

}

/*
 * put_color_pair
 * �����ʸ���Υ��顼�ڥ��ֹ����Ϥ���
 */
int
put_colorpair_number(char ch)
{
    return ch_attr[(signed) ch];
}

/*
 * get_color_pair
 * �����ʸ����Υ��顼�ڥ��ֹ�����Ϥ���
 */
void
get_colorpair_number(char ch, int num)
{
    ch_attr[(signed) ch] = num;
}

/*
 * addch_rogue
 * addch �Υ�åѡ��ؿ�
 * ʸ����Υ��顼ɽ�����礷�������餦
 */
int
addch_rogue(const chtype ch)
{
#if defined( COLOR )
    attrset(COLOR_PAIR(put_colorpair_number(ch)));
#endif /* COLOR */
    return addch(ch);
}

/*
 * mvaddch_rogue
 * mvaddch �Υ�åѡ��ؿ�
 * ʸ����Υ��顼ɽ�����礷�������餦
 */
int
mvaddch_rogue(int y, int x, const chtype ch)
{
#if defined( COLOR )
    attrset(COLOR_PAIR(put_colorpair_number(ch)));
#endif /* COLOR */
    return mvaddch(y, x, ch);
}

/*
 * addstr_rogue
 * addstr �Υ�åѡ��ؿ�
 * ʸ����Υ��顼ɽ�����礷�������餦
 */
int
addstr_rogue(const char *str)
{
#if defined( COLOR )
    attrset(COLOR_PAIR(0));
#endif /* COLOR */
    return addstr(str);
}

/*
 * mvaddstr_rogue
 * mvaddstr �Υ�åѡ��ؿ�
 * ʸ����Υ��顼ɽ�����礷�������餦
 */
int
mvaddstr_rogue(int y, int x, const char *str)
{
#if defined( COLOR )
    attr_t attr_stats;
    int color_stats;

    attr_get(&attr_stats, &color_stats, NULL);
    if (attr_stats & A_REVERSE) {
	attrset(COLOR_PAIR(CYAN));
	attron(A_REVERSE);
    } else if (strcmp(str, mesg[187]) == 0) {
	attrset(COLOR_PAIR(YELLOW));
    } else if (strcmp(str, mesg[188]) == 0) {
	attrset(COLOR_PAIR(GREEN));
    } else {
	attrset(COLOR_PAIR(0));
    }
#endif /* COLOR */
    return mvaddstr(y, x, str);
}

/*
 * mvinch_rogue
 * mvinch �Υ�åѡ��ؿ�
 * °��������ʸ�������������Ф���
 */
chtype
mvinch_rogue(int y, int x)
{
#if defined( COLOR )
    return mvinch(y, x) & A_CHARTEXT;
#else /* not COLOR */
    return mvinch(y, x);
#endif /* not COLOR */
}
