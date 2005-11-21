/*
 * machdep.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

/* Included in this file are all system dependent routines.  Extensive use
 * of #ifdef's will be used to compile the appropriate code on each system:
 *
 *    UNIX:        all UNIX systems.
 *    UNIX_BSD4_2: UNIX BSD 4.2 and later, UTEK, (4.1 BSD too?)
 *    UNIX_SYSV:   UNIX system V
 *    UNIX_V7:     UNIX version 7
 *
 * All UNIX code should be included between the single "#ifdef UNIX" at the
 * top of this file, and the "#endif UNIX" at the bottom.
 * 
 * To change a routine to include a new UNIX system, simply #ifdef the
 * existing routine, as in the following example:
 *
 *   To make a routine compatible with UNIX system V, change the first
 *   function to the second:
 *
 *      md_function()
 *      {
 *         code;
 *      }
 *
 *      md_function()
 *      {
 *      #ifdef UNIX_SYSV
 *         sysVcode;
 *      #else
 *         code;
 *      #endif / * UNIX_SYSV * /
 *      }
 *
 * Appropriate variations of this are of course acceptible.
 * The use of "#elseif" is discouraged because of non-portability.
 * If the correct #define doesn't exist, "UNIX_SYSV" in this case, make it up
 * and insert it in the list at the top of the file.  Alter the CFLAGS
 * in you Makefile appropriately.
 *
 */

#include <stdio.h>
#ifdef UNIX
#  include <sys/types.h>
#  include <sys/file.h>
#  include <sys/stat.h>
#  ifdef UNIX_SYSV
#    include <time.h>
#    include <termio.h>
#  endif /* UNIX_SYSV */
#  ifdef UNIX_BSD4_2
#    include <sys/time.h>
#    include <sgtty.h>
#  endif /* UNIX_BSD4_2 */
#endif /* UNIX */
#ifdef HUMAN
#include <doslib.h>	/* by Yasha */
#include <conio.h>	/* by Yasha */
#include <time.h>	/* by Yasha */
#include <direct.h>	/* by Yasha */
#include <signal.h>	/* by Yasha */
#include <stat.h>	/* by Yasha */
#else
#ifdef MSDOS
#  ifdef LC4
#    include <fcntl.h>
#  else
#    if !defined(__TURBOC__) || __TURBOC__ >= 0x0200
#      include <sys\types.h>
#    endif /* __TURBOC__ */
#    include <sys\stat.h>
#  endif /* LC4 */
#  include <time.h>
#  include <dos.h>
#endif /* MSDOS */
#endif /* HUMAN */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "rogue.h"
#include "machdep.h"
#include "init.h"

#ifdef HUMAN
getchar()		/* by Yasha */
{			/* by Yasha */
	return INKEY();	/* by Yasha */
}			/* by Yasha */
#endif
#if defined(MSDOS) && !defined(HUMAN)	/* by Yasha */
/*#ifdef MSDOS*/
getchar()
{
	register int c;

	/*
	 * hide cursor, and do console input
	 */
	putstr(cursor_on);
#ifdef __TURBOC__
	_AH = 0x07;
	geninterrupt(0x21);
	c = _AL;
#else
	c = bdos(7, 0, 0) & 0xff;
#endif
	putstr(cursor_off);
	return (c);
}

putchar(c)
	int c;
{
#ifdef __TURBOC__
	_AH = 0x02;
	_AL = c;
	geninterrupt(0x29);
#else /* MS-C */
	union REGS regs;
	regs.h.ah = 0x02;
	regs.h.al = c;
	int86(0x29, &regs, &regs);
#endif
}

putstr(s)
	register char *s;
{
#ifdef __TURBOC__
	while (*s) {
		_AH = 0x02;
		_AL = *s++;
		geninterrupt(0x29);
	}
#else /* MS-C */
	union REGS regs;
	while (*s) {
		regs.h.ah = 0x02;
		regs.h.al = *s++;
		int86(0x29, &regs, &regs);
	}
#endif
}

#else
void
putstr(register char *s)
{
	while (*s)
		putchar(*s++);
}
#endif

#ifndef ORIGINAL
/*
 * md_getcwd:
 *
 * get full pathname of current directory and place it into dir.
 * maximum length of pathname is given as len.
 * return dir on success, or NULL on failure.
 */

char *
md_getcwd(char *dir, int len)
{
#ifdef UNIX
#ifdef UNIX_BSD4_2
	char *getwd();

	return (getwd(dir));
#else
	char *getcwd();

	return (getcwd(dir, len));
#endif /* UNIX_BSD4_2 */
#else /* UNIX */
#ifdef LC4
	return ((char *)getcwd(dir, len));
#else
	char *getcwd();

	return (getcwd(dir, len));
#endif /* LC4 */
#endif /* UNIX_BSD_4_2 */
}
#endif /* UNIX */

#ifndef ORIGINAL
/*
 * md_chdir:
 *
 * change directory to dir.  also change drive in MSDOS environment.
 * return 0 on success, or -1 on failure.
 */

int
md_chdir(char *dir)
{
#if defined(UNIX) || defined(HUMAN)
/*#ifdef UNIX*/
	return (chdir(dir));
#endif /* UNIX */

#if defined(MSDOS) && !defined(HUMAN)
/*#ifdef MSDOS*/
	chdrive(dir);
	return (chdir(dir));
#endif /* MSDOS */
}
#endif /* ORIGINAL */

#if defined(MSDOS) && !defined(HUMAN)
/*#ifdef MSDOS*/
/*
 * chdrive:
 *
 * change the default drive to drive.
 */
#define SELECTDISK	0x0E
chdrive(dir)
char *dir;
{
	char *p, *strchr();
	char drive;
#ifndef __TURBOC__
	union REGS regs;
#endif

	p = strchr(dir, ':');
	if (p == NULL)
		return;
	p--;
	drive = (*p >= 'A' && *p <= 'Z')? *p - 'A': *p - 'a';
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
#endif /* MSDOS */

/* md_slurp:
 *
 * This routine throws away all keyboard input that has not
 * yet been read.  It is used to get rid of input that the user may have
 * typed-ahead.
 *
 * This function is not necessary, so it may be stubbed.  The might cause
 * message-line output to flash by because the game has continued to read
 * input without waiting for the user to read the message.  Not such a
 * big deal.
 */

void
md_slurp(void)
{
#ifdef UNIX
	long ln = 0;

#ifdef UNIX_BSD4_2
	ioctl(0, FIONREAD, &ln);
#endif /* UNIX_BSD4_2 */
#ifdef UNIX_SYSV
	ioctl(0, TCFLSH, &ln);
	ln = 0;
#endif /* UNIX_SYSV */
#ifdef UNIX_386BSD
	fpurge(stdin);
#else
#ifdef UNIX_SYSV
	fflush(stdin);
#else
	ln += stdin->_cnt;
	for (; ln > 0; ln--) {
		(void) getchar();
	}
#endif /* UNIX_SYSV */
#endif /*386BSD*/
#endif /* UNIX */

#ifdef HUMAN
	while (kbhit())		/* by Yasha */
		INKEY();	/* by Yasha */
#else
#ifdef MSDOS
#ifdef __TURBOC__
	while (_AH = 0x0b, geninterrupt(0x21), _AL) {
		_AH = 0x07;
		geninterrupt(0x21);
	}
#else
	while (kbhit())
		bdos(7, 0, 0);
#endif
#endif
#endif	/* HUMAN */
}

#ifndef MSDOS
/* md_control_keyboard():
 *
 * This routine is much like md_cbreak_no_echo_nonl() below.  It sets up the
 * keyboard for appropriate input.  Specifically, it prevents the tty driver
 * from stealing characters.  For example, ^Y is needed as a command
 * character, but the tty driver intercepts it for another purpose.  Any
 * such behavior should be stopped.  This routine could be avoided if
 * we used RAW mode instead of CBREAK.  But RAW mode does not allow the
 * generation of keyboard signals, which the program uses.
 *
 * The parameter 'mode' when true, indicates that the keyboard should
 * be set up to play rogue.  When false, it should be restored if
 * necessary.
 *
 * This routine is not strictly necessary and may be stubbed.  This may
 * cause certain command characters to be unavailable.
 */

void
md_control_keyboard(boolean mode)
{
#ifdef UNIX
	static boolean called_before = 0;
#ifdef UNIX_BSD4_2
	static struct ltchars ltc_orig;
	static struct tchars tc_orig;
	struct ltchars ltc_temp;
	struct tchars tc_temp;
#endif /* UNIX_BSD4_2 */
#ifdef UNIX_SYSV
	static struct termio _oldtty;
	struct termio _tty;
#endif /* UNIX_SYSV */

	if (!called_before) {
		called_before = 1;
#ifdef UNIX_BSD4_2
		ioctl(0, TIOCGETC, &tc_orig);
		ioctl(0, TIOCGLTC, &ltc_orig);
#endif /* UNIX_BSD4_2 */
#ifdef UNIX_SYSV
		ioctl(0, TCGETA, &_oldtty);
#endif /* UNIX_SYSV */
	}
#ifdef UNIX_BSD4_2
	ltc_temp = ltc_orig;
	tc_temp = tc_orig;
#endif /* UNIX_BSD4_2 */
#ifdef UNIX_SYSV
	_tty = _oldtty;
#endif /* UNIX_SYSV */

	if (!mode) {
#ifdef UNIX_BSD4_2
		ltc_temp.t_suspc = ltc_temp.t_dsuspc = -1;
		ltc_temp.t_rprntc = ltc_temp.t_flushc = -1;
		ltc_temp.t_werasc = ltc_temp.t_lnextc = -1;
		tc_temp.t_startc = tc_temp.t_stopc = -1;
#endif /* UNIX_BSD4_2 */
#ifdef UNIX_SYSV
		/* _tty.c_cc[VSWTCH] = CNSWTCH; */
#endif /* UNIX_SYSV */
	}
#ifdef UNIX_BSD4_2
	ioctl(0, TIOCSETC, &tc_temp);
	ioctl(0, TIOCSLTC, &ltc_temp);
#endif /* UNIX_BSD4_2 */
#ifdef UNIX_SYSV
	ioctl(0, TCSETA, &_tty);
#endif /* UNIX_SYSV */
#endif /* UNIX */
}
#endif /*MSDOS*/

/* md_heed_signals():
 *
 * This routine tells the program to call particular routines when
 * certain interrupts/events occur:
 *
 *      SIGINT: call onintr() to interrupt fight with monster or long rest.
 *      SIGQUIT: call byebye() to check for game termination.
 *      SIGHUP: call error_save() to save game when terminal hangs up.
 *
 *		On VMS, SIGINT and SIGQUIT correspond to ^C and ^Y.
 *
 * This routine is not strictly necessary and can be stubbed.  This will
 * mean that the game cannot be interrupted properly with keyboard
 * input, this is not usually critical.
 */

void
md_heed_signals(void)
{
#ifdef UNIX
void onintr(int);
	signal(SIGINT, onintr);
	signal(SIGQUIT, byebye);
	signal(SIGHUP, error_save);
#endif /* UNIX */

#ifdef HUMAN
	signal(SIGINT, onintr);
#else
#ifdef MSDOS
#if defined(__TURBOC__) && __TURBOC__ < 0x0200
	ctrlbrk(onintr);
#else
	signal(SIGINT, onintr);
#endif /* __TURBOC__ */
#endif
#endif	/* HUMAN */
}

/* md_ignore_signals():
 *
 * This routine tells the program to completely ignore the events mentioned
 * in md_heed_signals() above.  The event handlers will later be turned on
 * by a future call to md_heed_signals(), so md_heed_signals() and
 * md_ignore_signals() need to work together.
 *
 * This function should be implemented or the user risks interrupting
 * critical sections of code, which could cause score file, or saved-game
 * file, corruption.
 */

void
md_ignore_signals(void)
{
#ifdef UNIX
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
#endif /* UNIX */

#ifdef HUMAN
	signal(SIGINT, SIG_IGN);	/* by Yasha */
#else
#ifdef MSDOS
#if defined(__TURBOC__) && __TURBOC__ < 0x0200
	ctrlbrk(ignintr);
#else
	signal(SIGINT, SIG_IGN);
#endif /* __TURBOC__ */
#endif
#endif	/* HUMAN */
}

/* md_get_file_id():
 *
 * This function returns an integer that uniquely identifies the specified
 * file.  It need not check for the file's existence.  In UNIX, the inode
 * number is used.
 *
 * This function need not be implemented.  To stub the routine, just make
 * it return 0.  This will make the game less able to prevent users from
 * modifying saved-game files.  This is probably no big deal.
 */

int
md_get_file_id(char *fname)
{
#ifdef UNIX
	struct stat sbuf;

	if (stat(fname, &sbuf)) {
		return(-1);
	}
	return((int) sbuf.st_ino);
#endif /* UNIX */

#ifdef MSDOS
	return (0);
#endif /* MSDOS */
}

/* md_link_count():
 *
 * This routine returns the number of hard links to the specified file.
 *
 * This function is not strictly necessary.  On systems without hard links
 * this routine can be stubbed by just returning 1.
 */

#ifndef MSDOS
int
md_link_count(char *fname)
{
#ifdef UNIX
	struct stat sbuf;

	stat(fname, &sbuf);
	return((int) sbuf.st_nlink);
#endif /* UNIX */
}
#endif /* MSDOS */

/* md_gct(): (Get Current Time)
 *
 * This function returns the current year, month(1-12), day(1-31), hour(0-23),
 * minute(0-59), and second(0-59).  This is used for identifying the time
 * at which a game is saved.
 *
 * This function is not strictly necessary.  It can be stubbed by returing
 * zeros instead of the correct year, month, etc.  If your operating
 * system doesn't provide all of the time units requested here, then you
 * can provide only those that it does, and return zeros for the others.
 * If you cannot provide good time values, then users may be able to copy
 * saved-game files and play them.  
 */

void
md_gct(struct rogue_time *rt_buf)
{
	struct tm *t, *localtime();
	long seconds;

	time(&seconds);
	t = localtime(&seconds);

	rt_buf->year = t->tm_year;
	rt_buf->month = t->tm_mon + 1;
	rt_buf->day = t->tm_mday;
	rt_buf->hour = t->tm_hour;
	rt_buf->minute = t->tm_min;
	rt_buf->second = t->tm_sec;
}

/* md_gfmt: (Get File Modification Time)
 *
 * This routine returns a file's date of last modification in the same format
 * as md_gct() above.
 *
 * This function is not strictly necessary.  It is used to see if saved-game
 * files have been modified since they were saved.  If you have stubbed the
 * routine md_gct() above by returning constant values, then you may do
 * exactly the same here.
 * Or if md_gct() is implemented correctly, but your system does not provide
 * file modification dates, you may return some date far in the past so
 * that the program will never know that a saved-game file being modified.  
 * You may also do this if you wish to be able to restore games from
 * saved-games that have been modified.
 */

void
md_gfmt(char *fname, struct rogue_time *rt_buf)
{
#ifdef LC4
	int fd;
	long ft;
	char s[6];

	fd = open(fname, O_RDONLY|O_RAW);
	ft = getft(fd);
	close(fd);
	ftunpk(ft, s);
	rt_buf->year = s[0] + 80;
	rt_buf->month = s[1];
	rt_buf->day = s[2];
	rt_buf->hour = s[3];
	rt_buf->minute = s[4];
	rt_buf->second = s[5];
#else
	struct stat sbuf;
	long seconds;
	struct tm *t;

	stat(fname, &sbuf);
	seconds = (long) sbuf.st_mtime;
	t = localtime(&seconds);

#if defined(__TURBOC__) && __TURBOC__ < 0x0200
	/*
	 * Time routines of Turbo C 1.5J (both from MSA, SPL)
	 * has not been modified for Japanese use.
	 * So we must check the daylight saving time flag,
	 * and then re-correct the time.
	 */
	if (t->tm_isdst) {
		seconds -= 3600;
		t = localtime(&seconds);
	}
#endif

	rt_buf->year = t->tm_year;
	rt_buf->month = t->tm_mon + 1;
	rt_buf->day = t->tm_mday;
	rt_buf->hour = t->tm_hour;
	rt_buf->minute = t->tm_min;
	rt_buf->second = t->tm_sec;
#endif /* !LC4 */
}

/* md_df: (Delete File)
 *
 * This function deletes the specified file, and returns true (1) if the
 * operation was successful.  This is used to delete saved-game files
 * after restoring games from them.
 *
 * Again, this function is not strictly necessary, and can be stubbed
 * by simply returning 1.  In this case, saved-game files will not be
 * deleted and can be replayed.
 */

boolean
md_df(char *fname)
{
	if (unlink(fname)) {
		return(0);
	}
	return(1);
}

/* md_gln: (Get login name)
 *
 * This routine returns the login name of the user.  This string is
 * used mainly for identifying users in score files.
 *
 * A dummy string may be returned if you are unable to implement this
 * function, but then the score file would only have one name in it.
 */

char *
md_gln(void)
{
#ifdef UNIX
	char *getlogin();
	char *t;
	char *md_getenv();	/* by Yasha */

	if ((t = md_getenv("FIGHTER")) == NULL)	/* by Yasha */
		if ((t = getlogin()) == NULL)	/* by Yasha */
			t = md_getenv("USER");	/* by Yasha */
/*	t = getlogin();*/	/* killed by Yasha */
	return(t);
#endif /* UNIX */

#ifdef MSDOS
	char *t;

	if ((t = md_getenv("FIGHTER")) == NULL)		/* by Yasha */
		if ((t = md_getenv("USER")) == NULL)	/* by Yasha */
/*	if ((t = md_getenv("USER")) == NULL)*/
#ifdef JAPAN
		t = "¿ÔªŒ";
#else
		t = "Fighter";
#endif
	return (t);
#endif /* MSDOS */
}

/* md_sleep:
 *
 * This routine causes the game to pause for the specified number of
 * seconds.
 *
 * This routine is not necessary at all, and can be stubbed with no ill
 * effects.
 */

void
md_sleep(int nsecs)
{
	(void) sleep(nsecs);
}
#ifdef HUMAN	/* by Yasha (till "#else") */
sleep(nsecs)
int nsecs;
{
	time_t t;
	if (nsecs < 1)
		nsecs = 1;
	t = time((time_t *) NULL) + (time_t) nsecs;
	while (time((time_t *) NULL) < t)
		;
}
#else
#ifdef MSDOS
#ifndef __TURBOC__
sleep(nsecs)
int nsecs;
{
	long t, time();

	if (nsecs < 1)
		nsecs = 1;
	t = time(0L) + (long)nsecs;
	while (time(0L) < t)
		;
}
#endif /* __TURBOC__ */
#endif /* MSDOS */
#endif /* HUMAN */

/* md_getenv()
 *
 * This routine gets certain values from the user's environment.  These
 * values are strings, and each string is identified by a name.  The names
 * of the values needed, and their use, is as follows:
 *
 *   TERMCAP
 *     The name of the users's termcap file, NOT the termcap entries
 *     themselves.  This is used ONLY if the program is compiled with
 *     CURSES defined (-DCURSES).  Even in this case, the program need
 *     not find a string for TERMCAP.  If it does not, it will use the
 *     default termcap file as returned by md_gdtcf();
 *   TERM
 *     The name of the users's terminal.  This is used ONLY if the program
 *     is compiled with CURSES defined (-DCURSES).  In this case, the string
 *     value for TERM must be found, or the routines in curses.c cannot
 *     function, and the program will quit.
 *   ROGUEOPTS
 *     A string containing the various game options.  This need not be
 *     defined.
 *   HOME
 *     The user's home directory.  This is only used when the user specifies
 *     '~' as the first character of a saved-game file.  This string need
 *     not be defined.
 *
 * If your system does not provide a means of searching for these values,
 * you will have to do it yourself.  None of the values above really need
 * to be defined except TERM when the program is compiled with CURSES
 * defined.  In this case, as a bare minimum, you can check the 'name'
 * parameter, and if it is "TERM" find the terminal name and return that,
 * else return zero.  If the program is not compiled with CURSES, you can
 * get by with simply always returning zero.  Returning zero indicates
 * that their is no defined value for the given string.
 */

char *
md_getenv(char *name)
{
	char *getenv();

	return getenv(name);
}

/* md_malloc()
 *
 * This routine allocates, and returns a pointer to, the specified number
 * of bytes.  This routines absolutely MUST be implemented for your
 * particular system or the program will not run at all.  Return zero
 * when no more memory can be allocated.
 */

char *
md_malloc(int n)
{
	void *malloc();

	return malloc(n);
}

/* md_gseed() (Get Seed)
 *
 * This function returns a seed for the random number generator (RNG).  This
 * seed causes the RNG to begin generating numbers at some point in it's
 * sequence.  Without a random seed, the RNG will generate the same set
 * of numbers, and every game will start out exactly the same way.  A good
 * number to use is the process id, given by getpid() on most UNIX systems.
 *
 * You need to find some single random integer, such as:
 *   process id.
 *   current time (minutes + seconds) returned from md_gct(), if implemented.
 *   
 * It will not help to return "get_rand()" or "rand()" or the return value of
 * any pseudo-RNG.  If you don't have a random number, you can just return 1,
 * but this means your games will ALWAYS start the same way, and will play
 * exactly the same way given the same input.
 */

int
md_gseed(void)
{
#ifdef UNIX
	return(getpid());
#endif /* UNIX */

#ifdef MSDOS
	long time();

	return ((int) time(0L));
#endif /* MSDOS */
}

/* md_exit():
 *
 * This function causes the program to discontinue execution and exit.
 * This function must be implemented or the program will continue to
 * hang when it should quit.
 */

void
md_exit(int status)
{
#ifndef ORIGINAL
	if (org_dir && *org_dir)
		md_chdir(org_dir);
#endif
	exit(status);
}

/* If you have a viable curses/termlib library, then use it and don't bother
 * implementing the routines below.  And don't compile with -DCURSES.
 */

#ifdef CURSES
#if defined(MSDOS) && !defined(HUMAN)	/* by Yasha */
/*#ifdef MSDOS*/

/*
 * << ioctl routine come from PC NETHACK >>
 *
 * Use the IOCTL DOS function call to change stdin and stdout to raw
 * mode.  For stdin, this prevents MSDOS from trapping ^P, thus
 * freeing us of ^P toggling 'echo to printer'.
 * Thanks to Mark Zbikowski (markz@microsoft.UUCP).
 */

#define DEVICE	0x80
#define RAW	0x20
#define IOCTL	0x44
#define GETBITS	0
#define SETBITS	1

static unsigned
ioctl(handle, mode, setvalue)
unsigned setvalue;
{
#ifdef __TURBOC__
	_AH = IOCTL;
	_AL = mode;
	_BX = handle;
	_DL = setvalue;
	_DH = 0;
	geninterrupt(0x21);
	return _DX;
#else
	union REGS regs;

	regs.h.ah = IOCTL;
	regs.h.al = mode;
	regs.x.bx = handle;
	regs.h.dl = setvalue;
	regs.h.dh = 0;			/* Zero out dh */
	intdos(&regs, &regs);
	return ((unsigned)regs.x.dx);
#endif
}
#endif /* MSDOS && !HUMAN */

/* md_cbreak_no_echo_nonl:
 *
 * This routine sets up some terminal characteristics.  The tty-driver
 * must be told to:
 *   1.)  Not echo input.
 *   2.)  Transmit input characters immediately upon typing. (cbreak mode)
 *   3.)  Move the cursor down one line, without changing column, and
 *        without generating a carriage-return, when it
 *        sees a line-feed.  This is only necessary if line-feed is ever
 *        used in the termcap 'do' (cursor down) entry, in which case,
 *        your system should must have a way of accomplishing this.
 *
 * When the parameter 'on' is true, the terminal is set up as specified
 * above.  When this parameter is false, the terminal is restored to the
 * original state.
 *
 * Raw mode should not to be used.  Keyboard signals/events/interrupts should
 * be sent, although they are not strictly necessary.  See notes in
 * md_heed_signals().
 *
 * This function must be implemented for rogue to run properly if the
 * program is compiled with CURSES defined to use the enclosed curses
 * emulation package.  If you are not using this, then this routine is
 * totally unnecessary.
 * 
 * Notice that information is saved between calls.  This is used to
 * restore the terminal to an initial saved state.
 *
 */

void
md_cbreak_no_echo_nonl(boolean on)
{
#ifdef UNIX
#ifdef UNIX_BSD4_2
	static struct sgttyb tty_buf;
	static int tsave_flags;

	if (on) {
		ioctl(0, TIOCGETP, &tty_buf);
		tsave_flags = tty_buf.sg_flags;
		tty_buf.sg_flags |= CBREAK;
		tty_buf.sg_flags &= ~(ECHO | CRMOD);	/* CRMOD: see note 3 above */
		ioctl(0, TIOCSETP, &tty_buf);
	} else {
		tty_buf.sg_flags = tsave_flags;
		ioctl(0, TIOCSETP, &tty_buf);
	}
#endif /* UNIX_BSD4_2 */
#ifdef UNIX_SYSV
	struct termio tty_buf;
	static struct termio tty_save;

	if (on) {
		ioctl(0, TCGETA, &tty_buf);
		tty_save = tty_buf;
		tty_buf.c_lflag &= ~(ICANON | ECHO);
		tty_buf.c_oflag &= ~ONLCR;
		tty_buf.c_cc[4] = 1;  /* MIN */
		tty_buf.c_cc[5] = 2;  /* TIME */
		ioctl(0, TCSETAF, &tty_buf);
	} else {
		ioctl(0, TCSETAF, &tty_save);
	}
#endif /* UNIX_SYSV */
#endif /* UNIX */

#if defined(MSDOS) && !defined(HUMAN)
/*#ifdef MSDOS*/
	static unsigned old0, old1;

	if (on) {
		old0 = ioctl(0, GETBITS, 0);
		if (old0 & DEVICE)
			ioctl(0, SETBITS, (old0 | RAW));
		old1 = ioctl(1, GETBITS, 0);
		if (old1 & DEVICE)
			ioctl(1, SETBITS, (old1 | RAW));
	} else {
		ioctl(0, SETBITS, (old0 & ~RAW));
		ioctl(1, SETBITS, (old1 & ~RAW));
	}
#endif /*MSDOS*/
}

/* md_gdtcf(): (Get Default Termcap File)
 *
 * This function is called ONLY when the program is compiled with CURSES
 * defined.  If you use your system's curses/termlib library, this function
 * won't be called.  On most UNIX systems, "/etc/termcap" suffices.
 *
 * If their is no such termcap file, then return 0, but in that case, you
 * must have a TERMCAP file returned from md_getenv("TERMCAP").  The latter
 * will override the value returned from md_gdtcf().  If the program is
 * compiled with CURSES defined, and md_gdtcf() returns 0, and
 * md_getenv("TERMCAP") returns 0, the program will have no terminal
 * capability information and will quit.
 */

#ifndef MSDOS
char *
md_gdtcf(void)
{
#ifdef UNIX
	return("/etc/termcap");
#endif /* UNIX */
}
#endif /* MSDOS */

/* md_tstp():
 *
 * This function puts the game to sleep and returns to the shell.  This
 * only applies to UNIX 4.2 and 4.3.  For other systems, the routine should
 * be provided as a do-nothing routine.  md_tstp() will only be referenced
 * in the code when compiled with CURSES defined.
 *
 */

#ifndef MSDOS
void
md_tstp(void)
{
#ifdef UNIX
#ifdef UNIX_BSD4_2
	kill(0, SIGTSTP);
#endif /* UNIX_BSD4_2 */
#endif /* UNIX */
}
#endif /* MSDOS */

#endif /* CURSES */
