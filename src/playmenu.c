/*
 * playmenu.c - player menu for rogue clone
 */

#include <stdio.h>
#include <dos.h>
#ifndef LC4
#include <process.h>
#endif

typedef unsigned char u_char;
typedef unsigned int u_int;
#define	char	u_char

char *strcpy(), *strncmp(), *strchr();
char *getenv(), *memset();

/*
 * General constant
 */
#define	NO	0
#define	YES	1

#define	OFF	0
#define	ON	1
#define	HIGH	2

/*
 * Character constant
 */
#define	SPACE	' '
#define	RET	'\r'
#define	NL	'\n'
#define	BS	'\b'
#define	ESCAPE	0x1b
#define CTRLU	0x15

typedef struct machine
{
    char *mtype, *cursor, *init, *term;
    int ltype;
} mac;

mac macs[] = {
    {"pc98", "1b5b3e356c/1b5b3e3568", "1b29301b5b3e336c1b5b3e3168", "1b5b3e316c", 2},
    {"pc100", "1b5b3e3568/1b5b3e356c", "1b29301b5b3d33681b5b3e316c", "1b5b3e3168", 0},
    {"ax", "1b5b3e356c/1b5b3e3568", "", "", 1},
    {"fmr", "1b5b3076/1b5b3176", "", "", 0},
    {"b16", "1b5b3e356c/1b5b3e3568", "", "", 0},
    {"if800", "1b31/1b30", "1b4e301b6e391b53202030", "1b53202031", 1},
    {"jis83", "", "", "", 1},
    {"", "", "", "", 0}
};

/*
 * Color constant
 */
#define	NONE		 0	/* dummy */
#define	WHITE		 1
#define	GREEN		 2
#define	YELLOW		 3
#define	MAGENTA		 4
#define	CYAN		 5
#define	RGREEN		 6
#define	RYELLOW		 7
#define	RMAGENTA 	 8
#define	RCYAN		 9
#define	NCOLORS		10

char *colorstr[2][NCOLORS] = {
    {"", "\033[m", "\033[m", "\033[m", "\033[m",
     "\033[m", "\033[7m", "\033[7m", "\033[7m", "\033[7m"},
    {"", "\033[m", "\033[32m", "\033[33m", "\033[35m",
     "\033[36m", "\033[32;7m", "\033[33;7m", "\033[35;7m", "\033[36;7m"}
};

/*
 * Line character index
 */
#define	HOR	0
#define	VERL	1
#define	VERR	2
#define	TOPL	3
#define	TOPR	4
#define	BOTL	5
#define	BOTR	6

char *linechar[][7] = {
    {"  ", " ", " ", "  ", " ", "  ", " "},
    {"¨¬", "¨­", "¨­", "¨®", "¨¯", "¨±", "¨°"},
    {"¬¥", "¬§", "¬§", "¬³", "¬·", "¬»", "¬¿"}
};

int linecolor[] = { RGREEN, GREEN, GREEN };

char curon[10], curoff[10], initstr[20], termstr[20];
char **linec;
char **colors = colorstr[YES];
int lcolor;

/*
 * Screen constant
 */
#define	LINES	24
#define	COLS	80
#define	heady	 2
#define	headx	 6
#define	playy	 7
#define	playx1	 6
#define	playx2	28
#define	menuy	 7
#define	menux	54
#define	msgy	19
#define	msgx	 6
#define	msglen	66
#define	barlen	18

#define	headbox()	box(heady-1, headx-4,  heady+1,  headx+msglen+2)
#define	playbox()	box(playy-2, playx1-4, playy+10, playx2+barlen+2)
#define	menubox()	box(menuy-2, menux-4,  menuy+10, menux+barlen+2)
#define	msgbox()	box(msgy-1,  msgx-4,   msgy+1,   msgx+msglen+2)

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

/*
 * Console i/o routines (catch no interrupts)
 */
xgetch()
{
#ifdef __TURBOC__
    _AH = 0x07;
    geninterrupt(0x21);
    return _AL;
#else
    return (bdos(7, 0, 0) & 0xff);
#endif
}

xputch(c)
{
#ifdef __TURBOC__
    _AH = 0x02;
    _AL = c;
    geninterrupt(0x29);
#else
    union REGS regs;
    regs.h.ah = 0x02;
    regs.h.al = c;
    int86(0x29, &regs, &regs);
#endif
}

xputs(s)
register char *s;
{
#ifndef __TURBOC__
    union REGS regs;
#endif

    while (*s) {
#ifdef __TURBOC__
	_AH = 0x02;
	_AL = *s++;
	geninterrupt(0x29);
#else
	regs.h.ah = 0x02;
	regs.h.al = *s++;
	int86(0x29, &regs, &regs);
#endif
    }
}

color(n)
int n;
{
    xputs(colors[n]);
}

move(y, x)
int y, x;
{
    char buf[10];

    sprintf(buf, "\033[%d;%dH", y + 1, x + 1);
    xputs(buf);
}

clear()
{
    xputs("\033[2J");
}

box(by, bx, ey, ex)
int by, bx, ey, ex;
{
    register int n;

    color(lcolor);
    move(by, bx);
    xputs(linec[TOPL]);
    for (n = bx + 2; n < ex; n += 2)
	xputs(linec[HOR]);
    xputs(linec[TOPR]);
    for (n = by + 1; n < ey; n++) {
	move(n, bx);
	xputs(linec[VERL]);
	move(n, ex);
	xputs(linec[VERR]);
    }
    move(ey, bx);
    xputs(linec[BOTL]);
    for (n = bx + 2; n < ex; n += 2)
	xputs(linec[HOR]);
    xputs(linec[BOTR]);
    color(WHITE);
}

/*
 * Footer messages
 */
#define	TITLE		 0
#define	WAIT		 1
#define	SELECT		 2
#define	ADD		 3
#define	ADDINPUT	 4
#define	ADDFULL		 5
#define	EMPTY		 6
#define	CHANGE		 7
#define	CHGINPUT	 8
#define	DELETE		 9
#define	DELASK		10
#define	DELSAVED	11
#define	NOEXEFILE	12
#define	BADUSR		13
#define	CANTWRITE	14
#define	INVALID1	15
#define	INVALID2	16
#define	SHOWCNTS	17
#define	BYEBYE		18
#define	NMESGS		19

char *msg[NMESGS] = {
    "¥í¡¼¥°¡¦¥¯¥í¡¼¥ó£²¡¡ÆüËÜ¸ìÈÇ£±¡¥£³¡¡¥×¥ì¥¤¥ä¡¼¡¦¥á¥Ë¥å¡¼£±¡¥£³",
    "¾¯¡¹¤ªÂÔ¤Á¤¯¤À¤µ¤¤ ...",
    "£È¡¢£Ê¡¢£Ë¡¢£Ì¤ÇÁªÂò¡¢¥¹¥Ú¡¼¥¹¤Ç¼Â¹Ô¤·¤Þ¤¹",
    "£È¡¢£Ê¡¢£Ë¡¢£Ì¤ÇÁªÂò¡¢¥¹¥Ú¡¼¥¹¤ÇÅÐÏ¿³«»Ï¡¢¥¨¥¹¥±¡¼¥×¤Ç¥­¥ã¥ó¥»¥ë",
    "¥×¥ì¥¤¥ä¡¼Ì¾¤òÆþÎÏ¤·¡¢¥ê¥¿¡¼¥ó¡¦¥­¡¼¤ò²¡¤·¤Æ¤¯¤À¤µ¤¤",
    "\004¥×¥ì¥¤¥ä¡¼¤Ï¤³¤ì°Ê¾åÅÐÏ¿¤Ç¤­¤Þ¤»¤ó¡¡\001¡Ê²¿¤«¥­¡¼¤ò²¡¤·¤Æ¤¯¤À¤µ¤¤¡Ë",
    "\004¥×¥ì¥¤¥ä¡¼¤Ï£±¿Í¤âÅÐÏ¿¤µ¤ì¤Æ¤¤¤Þ¤»¤ó¡¡\001¡Ê²¿¤«¥­¡¼¤ò²¡¤·¤Æ¤¯¤À¤µ¤¤¡Ë",
    "£È¡¢£Ê¡¢£Ë¡¢£Ì¤ÇÁªÂò¡¢¥¹¥Ú¡¼¥¹¤ÇÊÑ¹¹³«»Ï¡¢¥¨¥¹¥±¡¼¥×¤Ç¥­¥ã¥ó¥»¥ë",
    "¥×¥ì¥¤¥ä¡¼Ì¾¤òÊÑ¹¹¤·¡¢¥ê¥¿¡¼¥ó¡¦¥­¡¼¤ò²¡¤·¤Æ¤¯¤À¤µ¤¤",
    "£È¡¢£Ê¡¢£Ë¡¢£Ì¤ÇÁªÂò¡¢¥¹¥Ú¡¼¥¹¤Çºï½ü¡¢¥¨¥¹¥±¡¼¥×¤Ç¥­¥ã¥ó¥»¥ë",
    "ºï½ü¤·¤Æ¤è¤í¤·¤¤¤Ç¤¹¤«¡©¡¡\003¡Î£ù£î¡Ï",
    "¥²¡¼¥à¤¬¥»¡¼¥Ö¤µ¤ì¤Æ¤¤¤Þ¤¹¡¡¤½¤ì¤Ç¤âºï½ü¤·¤Þ¤¹¤«¡©¡¡\003¡Î£ù£î¡Ï",
    "\004£Ò£Ï£Ç£Õ£Å¡¥£Å£Ø£Å¤¬¼Â¹Ô¤Ç¤­¤Þ¤»¤ó¡¡\001¡Ê²¿¤«¥­¡¼¤ò²¡¤·¤Æ¤¯¤À¤µ¤¤¡Ë",
    "\004£Ò£Ï£Ç£Õ£Å¡¥£Õ£Ó£Ò¤¬Àµ¤·¤¯¤¢¤ê¤Þ¤»¤ó¡¡\001¡Êºï½ü¤·¤Þ¤¹¡Ë",
    "\004£Ò£Ï£Ç£Õ£Å¡¥£Õ£Ó£Ò¤Ë½ñ¤­¹þ¤á¤Þ¤»¤ó¡¡\001¡Ê²¿¤«¥­¡¼¤ò²¡¤·¤Æ¤¯¤À¤µ¤¤¡Ë",
    "¤½¤ó¤Ê¥­¡¼¤ò²¡¤·¤¿¤Ã¤Æ¡¢²¿¤âµ¯¤³¤ê¤Þ¤»¤ó¤è¡©",
    "²¿¤âµ¯¤³¤é¤Ê¤¤¤Ã¤Æ¤Ð¡ª¡¡¥³¥é¡ª",
    "¸«¤Ä¤«¤Ã¤Á¤ã¤Ã¤¿¡ª¡¡¥í¡¼¥°¤ÇÍ·¤ó¤À²ó¿ô¤Ç¤¹¡¡º¤¤Ã¤¿¤â¤ó¤À¡¢¥Û¥ó¥È¤Ë",
    "¤è¤«¤Ã¤¿¤é¡¢¤Þ¤¿Í·¤ó¤Ç¤¯¤À¤µ¤¤¤Í¡©¡¡¡¡\003 Presented by Junn Ohta 1989"
};

header()
{
    move(heady, headx);
    color(YELLOW);
    xputs(msg[TITLE]);
    color(WHITE);
}

message(which)
int which;
{
    register char *p;
    register int i;
    int n;
    static int lastlen = 0;

    n = lastlen;
    move(msgy, msgx);
    i = 0;
    for (p = msg[which]; *p; p++) {
	if (*p < NCOLORS) {
	    color(*p);
	    continue;
	}
	xputch(*p);
	i++;
    }
    lastlen = i;
    color(WHITE);
    for (; i < n; i++)
	xputch(' ');
}

/*
 * Player entry (also in file)
 */
#define	NPLAYERS	10
#define	NAMELEN		14

u_int pcount[NPLAYERS];
char pname[NPLAYERS][NAMELEN + 1];
char ppass[NPLAYERS][NAMELEN + 1];

/*
 * Selection and connection table structure
 */
typedef struct _stable
{
    int cmd;			/* command to be execute */
    char *msg, *name;		/* message and name on screen */
    int saved;			/* saved game exists? (player only) */
    int hidden;			/* hidden command? */
    int y, x;			/* screen location */
} stable;

typedef struct _ctable
{
    int up;
    int down;
    int left;
    int right;
} ctable;

/*
 * Size of stable and ctable
 */
#define	NCOMMANDS	15

/*
 * Value of stable.cmd
 */
#define	CMDPLAY		 0
#define	CMDSCORE	 1
#define	CMDADD		 2
#define	CMDCHANGE 	 3
#define	CMDDELETE	 4
#define	CMDEXIT		 5
#define	CMDCOUNT	 6	/* hidden command (show play count) */

/*
 * Command menu
 */
stable menu[NCOMMANDS + 1] = {
/*	      cmd,      msg,     name,      saved, hidden,    y,      x */
    {CMDPLAY, "£°¡¥", pname[0], NO, NO, playy, playx1},
    {CMDPLAY, "£±¡¥", pname[1], NO, NO, playy + 2, playx1},
    {CMDPLAY, "£²¡¥", pname[2], NO, NO, playy + 4, playx1},
    {CMDPLAY, "£³¡¥", pname[3], NO, NO, playy + 6, playx1},
    {CMDPLAY, "£´¡¥", pname[4], NO, NO, playy + 8, playx1},
    {CMDPLAY, "£µ¡¥", pname[5], NO, NO, playy, playx2},
    {CMDPLAY, "£¶¡¥", pname[6], NO, NO, playy + 2, playx2},
    {CMDPLAY, "£·¡¥", pname[7], NO, NO, playy + 4, playx2},
    {CMDPLAY, "£¸¡¥", pname[8], NO, NO, playy + 6, playx2},
    {CMDPLAY, "£¹¡¥", pname[9], NO, NO, playy + 8, playx2},
    {CMDSCORE, "£Ó¡¥", "¥¹¥³¥¢¤ò¸«¤ë", NO, NO, menuy, menux},
    {CMDADD, "£Á¡¥", "¥×¥ì¥¤¥ä¡¼ÅÐÏ¿", NO, NO, menuy + 2, menux},
    {CMDCHANGE, "£Ã¡¥", "¥×¥ì¥¤¥ä¡¼ÊÑ¹¹", NO, NO, menuy + 4, menux},
    {CMDDELETE, "£Ä¡¥", "¥×¥ì¥¤¥ä¡¼ºï½ü", NO, NO, menuy + 6, menux},
    {CMDEXIT, "£Å¡¥", "½ªÎ»", NO, NO, menuy + 8, menux},
    {CMDCOUNT, "¡©¡¥", "¥×¥ì¥¤²ó¿ôÉ½¼¨", NO, YES, 0, 0}
};

#define	absent(n)	(menu[n].name[0] == 0)	/* should be skipped? */

/*
 * Characters for selection
 */
char *menuc = "0123456789SACDE?";

/*
 * Command connection table
 */
ctable ccon[NCOMMANDS] = {
    {14, 1, 14, 5},
    {0, 2, 10, 6},
    {1, 3, 11, 7},
    {2, 4, 12, 8},
    {3, 5, 13, 9},
    {4, 6, 0, 10},
    {5, 7, 1, 11},
    {6, 8, 2, 12},
    {7, 9, 3, 13},
    {8, 10, 4, 14},
    {9, 11, 5, 1},
    {10, 12, 6, 2},
    {11, 13, 7, 3},
    {12, 14, 8, 4},
    {13, 0, 9, 0}
};
int curc = -1;			/* current index for ccon */

/*
 * Player connection table
 */
ctable pcon[NPLAYERS] = {
    {9, 1, 9, 5},
    {0, 2, 5, 6},
    {1, 3, 6, 7},
    {2, 4, 7, 8},
    {3, 5, 8, 9},
    {4, 6, 0, 1},
    {5, 7, 1, 2},
    {6, 8, 2, 3},
    {7, 9, 3, 4},
    {8, 0, 4, 0}
};
int curp = -1;			/* current index for pcon */

showmenu()
{
    register int n;
    register stable *menup;

    for (n = 0; n < NCOMMANDS; n++) {
	menup = &menu[n];
	move(menup->y, menup->x);
	if (menup->saved) {
	    color(CYAN);
	}
	xputs(menup->msg);
	xputs(menup->name);
	if (menup->saved) {
	    color(WHITE);
	}
    }
}

/*
 * Reverse item on/off
 */
reverse(n, sw)
int n, sw;
{
    register stable *p;
    register int i;

    p = &menu[n];
    i = NAMELEN - strlen(p->name) + 2;
    move(p->y, p->x - 2);
    if (sw) {			/* (sw == ON || sw == HIGH) */
	color((sw == HIGH) ? RMAGENTA : (p->saved) ? RCYAN : RYELLOW);
	xputs("  ");
	xputs(p->msg);
	xputs(p->name);
	while (i--)
	    xputch(' ');
	color(WHITE);
    } else {
	xputs("  ");
	if (p->saved)
	    color(CYAN);
	xputs(p->msg);
	xputs(p->name);
	if (p->saved)
	    color(WHITE);
	while (i--)
	    xputch(' ');
    }
}

/*
 * Menu selection routine
 */
get()
{				/* get menu selection character */
    register int c;

    c = xgetch();
    while (kbhit())
	c = xgetch();
    if (c >= 'a' && c <= 'z')
	c += 'A' - 'a';
    return (c);
}

index(c)			/* get index from selection character */
int c;
{
    register int i;

    for (i = 0; menuc[i]; i++)
	if (c == menuc[i])
	    return (i);
    return (-1);
}

nplayers()
{
    register int i, n;

    n = 0;
    for (i = 0; i < NPLAYERS; i++)
	if (absent(i) == NO)
	    n++;
    return (n);
}

addcmdpos()
{
    register int n;

    for (n = 0; n < NCOMMANDS; n++)
	if (menu[n].cmd == CMDADD)
	    return (n);
    return (0);
}

#define	UP	     0x0b	/* sent from PC-9801 up arrow key */
#define	DOWN	     0x0a	/* sent from PC-9801 down arrow key */
#define	LEFT	     0x08	/* sent from PC-9801 left arrow key */
#define	RIGHT	     0x0c	/* sent from PC-9801 right arrow key */

cselect()
{
    register int c, n;
    int msgno;

    if (absent(curc)) {
	if (nplayers() == 0)
	    curc = addcmdpos();
	while (absent(curc))
	    curc = ccon[curc].down;
    }
    if (curp >= 0) {
	reverse(curp, OFF);
	curp = -1;
    }
    reverse(curc, ON);
    msgno = SELECT;
    for (;;) {
	message(msgno);
	c = get();
	if ((n = index(c)) >= 0) {
	    if (absent(n))
		continue;
	    if (menu[n].hidden == NO) {
		reverse(curc, OFF);
		curc = n;
		reverse(curc, ON);
	    }
	    return (n);
	}
	switch (c) {
	case 'H':
	case 'J':
	case 'K':
	case 'L':
	case UP:
	case DOWN:
	case LEFT:
	case RIGHT:
	    reverse(curc, OFF);
	    do {
		switch (c) {
		case 'K':
		case UP:
		    curc = ccon[curc].up;
		    break;
		case 'J':
		case DOWN:
		    curc = ccon[curc].down;
		    break;
		case 'H':
		case LEFT:
		    curc = ccon[curc].left;
		    break;
		case 'L':
		case RIGHT:
		    curc = ccon[curc].right;
		    break;
		}
	    } while (absent(curc));
	    reverse(curc, ON);
	    break;
	case RET:
	case SPACE:
	    return (curc);
	default:
	    msgno = (msgno != INVALID1) ? INVALID1 : INVALID2;
	    continue;
	}
	msgno = SELECT;
    }
}

#define	PRESENT		 0	/* skip absent entry */
#define	ABSENT		 1	/* skip present entry */

#define	digit(c)	((c) >= '0' && (c) <= '9')
#define	dignum(c)	((c) - '0')

/*
 * value of pselect
 */
#define	ESCAPED		-1
#define	NOTFOUND	-2

pselect(init, mno, status)
int init, mno, status;
{
    register int c, n;
    int msgno;

    n = init;
    while (absent(init) != (status == ABSENT)) {
	init = pcon[init].down;
	if (init == n)
	    return (NOTFOUND);
    }
    if (curp >= 0 && curp != init) {
	reverse(curp, OFF);
	/* curp = -1; */
    }
    curp = init;
    /* reverse(curc, HIGH); */
    reverse(curp, ON);
    msgno = mno;
    for (;;) {
	message(msgno);
	c = get();
	if (c == ESCAPE) {
	    reverse(curp, OFF);
	    curp = init;
	    return (ESCAPED);
	}
	if (digit(c)) {
	    n = dignum(c);
	    if (absent(n) != (status == ABSENT))
		continue;
	    reverse(curp, OFF);
	    curp = n;
	    reverse(curp, ON);
	    return (n);
	}
	switch (c) {
	case 'H':
	case 'J':
	case 'K':
	case 'L':
	case UP:
	case DOWN:
	case LEFT:
	case RIGHT:
	    reverse(curp, OFF);
	    do {
		switch (c) {
		case 'K':
		case UP:
		    curp = pcon[curp].up;
		    break;
		case 'J':
		case DOWN:
		    curp = pcon[curp].down;
		    break;
		case 'H':
		case LEFT:
		    curp = pcon[curp].left;
		    break;
		case 'L':
		case RIGHT:
		    curp = pcon[curp].right;
		    break;
		}
	    } while (absent(curp) != (status == ABSENT));
	    reverse(curp, ON);
	    break;
	case RET:
	case SPACE:
	    return (curp);
	default:
	    break;
	}
	msgno = mno;
    }
}

/*
 * Get name input (returns static area)
 *
 * returns NULL if escaped, or length is zero
 */
#ifdef EUC
#define kanji1(c)	((c) & 0x80)	/* by Yasha */
#else /* Shift JIS */
#define	kanji1(c)	((c)>=0x81 && (c)<=0x9f || (c)>=0xe0 && (c)<=0xfc)
#endif
/*#define sjis1(c)	((c)>=0x81 && (c)<=0x9f || (c)>=0xe0 && (c)<=0xfc)*/

fillup(len)
register int len;
{
    color(RGREEN);
    xputch(' ');
    color(WHITE);
    len++;
    for (; len < NAMELEN; len++)
	xputch(' ');
    if (len <= NAMELEN)
	xputs("¡Ï");
}

char *
input(no, init)
int no;
char *init;
{
    register char *p;
    register int n;
    int len, y, x;
    int c, c1;
    char kanji[NAMELEN + 1];
    static char buf[NAMELEN + 1];

    n = 0;
    if (*init) {
	for (p = init; *p; p++) {
	    if (kanji1(*p) && p[1]) {	/* by Yasha */
/*			if (sjis1(*p) && p[1]) {*/
		kanji[n] = kanji[n + 1] = YES;
		buf[n] = *p++;
		buf[n + 1] = *p;
		n += 2;
	    } else {
		kanji[n] = NO;
		buf[n] = *p;
		n++;
	    }
	}
    }
    len = n;
    y = menu[no].y;
    x = menu[no].x + 4;
    move(y, x - 2);
    xputs("¡Î");
    for (n = 0; n < len; n++)
	xputch(buf[n]);
    fillup(len);

    for (;;) {
	c = xgetch();
	if (kanji1(c)) {	/* by Yasha */
/*		if (sjis1(c)) {*/
	    c1 = xgetch();
	    if (len >= NAMELEN - 1)
		continue;
	    buf[len] = c;
	    buf[len + 1] = c1;
	    kanji[len] = kanji[len + 1] = YES;
	    move(y, x + len);
	    xputch(c);
	    xputch(c1);
	    len += 2;
	    fillup(len);
	    continue;
	}
	switch (c) {
	case ESCAPE:
	    return (NULL);
	case NL:
	case RET:
	    for (;;) {
		if (len == 0)
		    break;
		if (kanji[len - 1] && buf[len - 2] == 0x81
		    && buf[len - 1] == 0x40) {
		    len -= 2;
		    continue;
		}
		if (kanji[len - 1] == NO && buf[len - 1] == ' ') {
		    len--;
		    continue;
		}
		break;
	    }
	    if (len == 0)
		return (NULL);
	    buf[len] = '\0';
	    return (buf);
	case CTRLU:
	    if (len == 0)
		continue;
	    len = 0;
	    move(y, x);
	    fillup(0);
	    break;
	case BS:
	    if (len == 0)
		continue;
	    if (kanji[len - 1] == YES)
		len -= 2;
	    else
		len--;
	    move(y, x + len);
	    fillup(len);
	    break;
	default:
	    if (c < 0x20 || c == 0x7f)
		continue;
	    if (len >= NAMELEN)
		continue;
	    buf[len] = c;
	    kanji[len] = NO;
	    move(y, x + len);
	    xputch(c);
	    len++;
	    fillup(len);
	    break;
	}
    }
}

/*
 * Change current directory
 */
#define	SELECTDISK	0x0e

changedir(dir)
char *dir;
{
    register char *p;
    char drive;
#ifndef __TURBOC__
    union REGS regs;
#endif

    if (p = strchr(dir, ':')) {
	p--;
	drive = (*p >= 'A' && *p <= 'Z') ? *p - 'A' : *p - 'a';
#ifdef __TURBOC__
	_AH = SELECTDISK;
	_DL = drive;
	geninterrupt(0x21);
#else
	regs.h.ah = SELECTDISK;
	regs.h.dl = drive;
	intdos(&regs, &regs);
#endif
    }
    chdir(dir);
}

/*
 * Miscelaneous file handling
 */
#define	DIRLEN		64
#define	USRFILE		"ROGUE.USR"
#define	EXEFILE		"ROGUE.EXE"

char orgdir[DIRLEN + 1];
char gamedir[DIRLEN + 1];
char usrpath[DIRLEN + 1];
char exepath[DIRLEN + 1];
int exist_gd;			/* game directory exists? */

/*
 * Look up saved file and set menu[*].saved
 */
char savename[] = "USER0?.SAV";
#define	SAVEX		5

look_saved()
{
    register int n;

    for (n = 0; n < NPLAYERS; n++) {
	savename[SAVEX] = n + '0';
	menu[n].saved = (access(savename, 0) == 0) ? YES : NO;
    }
}

/*
 * Read/write user file
 */
searchusr()
{
    register char *p, *q;

    if (exist_gd)
	p = gamedir;
    else if ((p = getenv("HOME")) == NULL)
	p = orgdir;
    q = usrpath;
    while (*p)
	*q++ = *p++;
    if (q[-1] != '\\')
	*q++ = '\\';
    strcpy(q, USRFILE);
}

readfile()
{
    register int n;
    register FILE *fp;

    if ((fp = fopen(usrpath, "rb")) == NULL) {
	for (n = 0; n < NPLAYERS; n++) {
	    pcount[n] = 0;
	    *pname[n] = 0;
	    *ppass[n] = 0;
	}
	return;
    }
    for (n = 0; n < NPLAYERS; n++) {
	fread((char *) &pcount[n], sizeof(int), 1, fp);
	fread(pname[n], NAMELEN + 1, 1, fp);
	fread(ppass[n], NAMELEN + 1, 1, fp);
    }
    fclose(fp);
    return;
}

writefile()
{
    register int n;
    register FILE *fp;

    if ((fp = fopen(usrpath, "wb")) == NULL) {
	message(CANTWRITE);
	xgetch();
	return;
    }
    for (n = 0; n < NPLAYERS; n++) {
	fwrite((char *) &pcount[n], sizeof(int), 1, fp);
	fwrite(pname[n], NAMELEN + 1, 1, fp);
	fwrite(ppass[n], NAMELEN + 1, 1, fp);
    }
    fclose(fp);
    return;
}

/*
 * Search exefile pathname
 */
searchexe()
{
    register char *p, *q;
    char buf[DIRLEN + 1];

    p = orgdir;
    q = buf;
    while (*p)
	*q++ = *p++;
    if (q[-1] != '\\')
	*q++ = '\\';
    strcpy(q, EXEFILE);

    if (access(buf, 0) == 0)
	goto currentdir;
    if ((p = getenv("PATH")) == NULL)
	goto currentdir;
    while (*p) {
	q = exepath;
	while (*p && *p != ';')
	    *q++ = *p++;
	if (q[-1] != '\\')
	    *q++ = '\\';
	strcpy(q, EXEFILE);
	if (access(exepath, 0) == 0)
	    return;
	if (*p == ';')
	    p++;
    }

  currentdir:
    strcpy(exepath, buf);
}

/*
 * Initialize and terminate
 */
char envname[] = "ROGUEOPT?";
#define	TAILX		     8
char *tail = "S123456789";

char envbuf[1024];

init()
{
    register char *p, *q;
    int i;
    int ltype;
    char *cursorp, *initp, *termp;
    mac *mp;
    char buf[128];

    getcwd(orgdir, DIRLEN);
    strcpy(gamedir, orgdir);
    exist_gd = NO;

    *envbuf = 0;
    for (q = tail; *q; q++) {
	envname[TAILX] = *q;
	p = getenv(envname);
	if (p == NULL)
	    continue;
	strcat(envbuf, ",");
	strcat(envbuf, p);
    }
    for (p = envbuf; *p; p++)
	if (*p >= 'A' && *p <= 'Z')
	    *p += 'a' - 'A';

    cursorp = initp = termp = "";
    ltype = 0;
    p = envbuf;
    while (*p) {
	if (*p == ',') {
	    p++;
	    continue;
	}
	q = buf;
	while (*p && *p != ',')
	    *q++ = *p++;
	*q = 0;
	if (!strcmp(buf, "color")) {
	    colors = colorstr[YES];
	} else if (!strcmp(buf, "nocolor")) {
	    colors = colorstr[NO];
	} else if (!strncmp(buf, "type:", 5)) {
	    q = buf + 5;
	    for (mp = macs; *mp->mtype; mp++)
		if (!strcmp(q, mp->mtype))
		    break;
	    cursorp = mp->cursor;
	    initp = mp->init;
	    termp = mp->term;
	    ltype = mp->ltype;
	} else if (!strncmp(buf, "cursor:", 7)) {
	    cursorp = buf + 7;
	} else if (!strncmp(buf, "init:", 5)) {
	    initp = buf + 5;
	} else if (!strncmp(buf, "term:", 5)) {
	    termp = buf + 5;
	} else if (!strncmp(buf, "dir", 3)) {
	    for (q = buf; *q && *q != ':'; q++)
		/* void */ ;
	    if (*q == ':' && *++q) {
		strcpy(gamedir, q);
		exist_gd = YES;
	    }
	}
    }

    linec = linechar[ltype];
    lcolor = linecolor[ltype];
    p = initp;
    q = initstr;
    while ((i = get_hex_num(p, 2)) >= 0) {
	*q++ = i;
	p += 2;
    }
    *q = 0;
    p = termp;
    q = termstr;
    while ((i = get_hex_num(p, 2)) >= 0) {
	*q++ = i;
	p += 2;
    }
    *q = 0;
    p = cursorp;
    q = curon;
    while ((i = get_hex_num(p, 2)) >= 0) {
	*q++ = i;
	p += 2;
    }
    *q = 0;
    if (*p)
	p++;
    q = curoff;
    while ((i = get_hex_num(p, 2)) >= 0) {
	*q++ = i;
	p += 2;
    }
    *q = 0;

    if (exist_gd == YES)
	changedir(gamedir);

    searchusr();
    searchexe();
}

term()
{
    if (exist_gd == YES)
	changedir(orgdir);

    exit(0);
}

/*
 * Set environment for user "no"
 */
setenv(no)
int no;
{
    register char *p;
    char fname[64];
    static char ubuf[16], buf[128];

    sprintf(ubuf, "USER=USER%d", no);
    putenv(ubuf);
    strcpy(fname, gamedir);
    for (p = fname; *p; p++)
	/* void */ ;
    if (p[-1] != '\\')
	*p++ = '\\';
    sprintf(p, "USER%02d.SAV", no);
    sprintf(buf, "ROGUEOPT9=name:%s,file:%s", pname[no], fname);
    putenv(buf);
}

/*
 * Add new entry
 */
add_entry(no, name)
register int no;
char *name;
{
    pcount[no] = 0;
    memset(pname[no], '\0', NAMELEN);
    memset(ppass[no], '\0', NAMELEN);
    strcpy(pname[no], name);
    menu[no].saved = NO;
    savename[SAVEX] = no + '0';
    if (access(savename, 0) == 0)
	unlink(savename);
}

/*
 * Change present entry
 */
change_entry(no, name)
register int no;
char *name;
{
    memset(pname[no], '\0', NAMELEN);
    strcpy(pname[no], name);
}

/*
 * Delete present entry
 */
delete_entry(no)
register int no;
{
    pcount[no] = 0;
    memset(pname[no], '\0', NAMELEN);
    memset(ppass[no], '\0', NAMELEN);
    menu[no].saved = NO;
    savename[SAVEX] = no + '0';
    if (access(savename, 0) == 0)
	unlink(savename);
}

/*
 * Add player command
 */
add_player()
{
    register int n;
    register char *p;

    n = 0;
    reverse(curc, HIGH);
    for (;;) {
	n = pselect(n, ADD, ABSENT);
	if (n == ESCAPED) {
	    reverse(curc, ON);
	    message(SELECT);
	    return;
	}
	if (n == NOTFOUND) {
	    message(ADDFULL);
	    xgetch();
	    reverse(curc, ON);
	    return;
	}
	message(ADDINPUT);
	reverse(n, OFF);
	p = input(n, "");
	if (p != NULL) {
	    add_entry(n, p);
	    reverse(n, ON);
	    message(WAIT);
	    writefile();
	    reverse(n, OFF);
	    reverse(curc, ON);
	    message(SELECT);
	    return;
	}
    }
}

/*
 * Change player command
 */
change_player()
{
    register int n;
    register char *p;

    n = 0;
    reverse(curc, HIGH);
    for (;;) {
	n = pselect(n, CHANGE, PRESENT);
	if (n == ESCAPED) {
	    reverse(curc, ON);
	    message(SELECT);
	    return;
	}
	if (n == NOTFOUND) {
	    message(EMPTY);
	    xgetch();
	    reverse(curc, ON);
	    return;
	}
	message(CHGINPUT);
	reverse(n, OFF);
	p = input(n, menu[n].name);
	if (p != NULL) {
	    change_entry(n, p);
	    reverse(n, ON);
	    message(WAIT);
	    writefile();
	    reverse(n, OFF);
	    reverse(curc, ON);
	    message(SELECT);
	    return;
	}
    }
}

/*
 * Delete player command
 */
#define	isyes(c)	((c) == 'y' || (c) == 'Y')
#define	isno(c)		((c) == 'n' || (c) == 'N' || (c) == ESCAPE)

delete_player()
{
    register int n, c;

    n = 0;
    reverse(curc, HIGH);
    for (;;) {
	n = pselect(n, DELETE, PRESENT);
	if (n == ESCAPED) {
	    reverse(curc, ON);
	    message(SELECT);
	    return;
	}
	if (n == NOTFOUND) {
	    message(EMPTY);
	    xgetch();
	    reverse(curc, ON);
	    return;
	}
	message(menu[n].saved ? DELSAVED : DELASK);
	do {
	    c = xgetch();
	} while (!isyes(c) && !isno(c));
	if (isyes(c)) {
	    delete_entry(n);
	    reverse(n, ON);
	    message(WAIT);
	    writefile();
	    reverse(n, OFF);
	    reverse(curc, ON);
	    message(SELECT);
	    return;
	}
    }
}

/*
 * Play rogue
 */
play(no)
register int no;
{
    register char *arg;
    int status;

    reverse(no, HIGH);
    message(WAIT);
    pcount[no]++;
    writefile();
    setenv(no);
    arg = menu[no].saved ? "-r" : NULL;
#ifdef LC4
    status = forkl(exepath, "rogue", arg, NULL);
#else
    status = spawnl(P_WAIT, exepath, "rogue", arg, NULL);
#endif
    if (status < 0) {
	message(NOEXEFILE);
	xgetch();
	reverse(no, ON);
	return;
    }
    initcrt();
    look_saved();
    showmenu();
    reverse(no, ON);
}

show_score()
{
    int status;

    reverse(curc, HIGH);
    message(WAIT);
    setenv(0);
#ifdef LC4
    status = forkl(exepath, "rogue", "-s", NULL);
#else
    status = spawnl(P_WAIT, exepath, "rogue", "-s", NULL);
#endif
    if (status < 0) {
	message(NOEXEFILE);
	xgetch();
	reverse(curc, ON);
	return;
    }
    xputs(curoff);
    move(20, 10);
    color(MAGENTA);
    xputs("¡á¥¹¥Ú¡¼¥¹¤ò²¡¤·¤Æ¤¯¤À¤µ¤¤¡á");
    color(WHITE);
    xgetch();
    initcrt();
    showmenu();
    reverse(curc, ON);
}

/*
 * Print zenkaku number
 */
char *dig[] = { "£°", "£±", "£²", "£³", "£´", "£µ", "£¶", "£·", "£¸", "£¹" };

znum(num, n)
int num, n;
{
    register char *p;
    char buf[20];

    sprintf(buf, "%*d", n, num);
    for (p = buf; *p; p++) {
	switch (*p) {
	case ' ':
	    xputs("  ");
	    break;
	case '-':
	    xputs("¡Ý");
	    break;
	default:
	    xputs(dig[*p - '0']);
	    break;
	}
    }
}

/*
 * Show counts command
 */
show_counts()
{
    register int n;
    register stable *p;

    message(SHOWCNTS);
    for (n = 0; n < NPLAYERS; n++) {
	p = &menu[n];
	move(p->y, p->x - 2);
	xputs("  ");
	if (p->saved)
	    color(CYAN);
	xputs(p->msg);
	znum(pcount[n], NAMELEN >> 1);
	if (p->saved)
	    color(WHITE);
	xputs("  ");
    }
    xgetch();
    for (n = 0; n < NPLAYERS; n++)
	reverse(n, OFF);
    message(SELECT);
}

/*
 * Quit command
 */
quit()
{
    reverse(curc, HIGH);
    message(BYEBYE);
    endcrt();
    term();
}

/*
 * Screen initializing and terminating routines
 */
initcrt()
{
    xputs(curoff);
    clear();
    xputs(initstr);
    color(WHITE);
    headbox();
    playbox();
    menubox();
    msgbox();
    header();
    message(WAIT);
    showmenu();
}

endcrt()
{
    xputs(termstr);
    move(LINES - 2, 0);
    xputs(curon);
}

/*
 * Main routine for multi-player frontend
 */
main()
{
    register int n;

    init();
    initcrt();
    readfile();
    look_saved();
    showmenu();
    curc = 0;
    for (;;) {
	n = cselect();
	switch (menu[n].cmd) {
	case CMDPLAY:
	    play(n);
	    break;
	case CMDSCORE:
	    show_score();
	    break;
	case CMDADD:
	    add_player();
	    break;
	case CMDCHANGE:
	    change_player();
	    break;
	case CMDDELETE:
	    delete_player();
	    break;
	case CMDEXIT:
	    /*
	     * no return
	     */
	    quit();
	case CMDCOUNT:
	    show_counts();
	    break;
	}
    }
}
