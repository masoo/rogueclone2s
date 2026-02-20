/*
 * playmenu.c
 * ncurses ベースの起動メニュー
 */

#include <curses.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "utf8.h"

#include "display.h"
#include "message.h"
#include "playmenu.h"
#include "rogue.h"

extern char *save_file;

/* メッセージ番号 */
#define MENU_TITLE 494
#define MENU_NEW 495
#define MENU_RESTORE 496
#define MENU_SCORE 497
#define MENU_EXIT 498
#define MENU_HELP 499
#define MENU_NO_SAVE 500
#define MENU_INVALID1 501
#define MENU_INVALID2 502

/* メニュー項目数 */
#define MENU_ITEMS 4

/* フッターメッセージ行 */
#define MSG_ROW 18

/* メニュー内部で使う仮想キーコード */
#define MK_UP (-1)
#define MK_DOWN (-2)
#define MK_NONE (-3)

/* メニュー項目の行位置 */
static const int item_row[MENU_ITEMS] = {9, 11, 13, 15};

/* メニュー項目のメッセージ番号 */
static const int item_msg[MENU_ITEMS] = {MENU_NEW, MENU_RESTORE, MENU_SCORE,
	MENU_EXIT};

/* ダイレクトキー */
static const char item_key[MENU_ITEMS] = {'N', 'R', 'S', 'Q'};

/* ダイレクトキー（全角表示用プレフィックス） */
static const char *item_prefix[MENU_ITEMS] = {"Ｎ．", "Ｒ．", "Ｓ．", "Ｑ．"};

/* 接続テーブル（上下巡回） */
static const int conn_up[MENU_ITEMS] = {3, 0, 1, 2};
static const int conn_down[MENU_ITEMS] = {1, 2, 3, 0};

/*
 * menu_getchar
 * メニュー用キー入力
 * ESC シーケンス（矢印キー）を解釈し、仮想キーコードを返す
 */
static int
menu_getchar(void)
{
	int ch;

	ch = rgetchar();

	if (ch != '\033')
		return ch;

	/* ESC の後に '[' が続くか確認（ノンブロッキング） */
	nodelay(stdscr, TRUE);
	int ch2 = getch();
	nodelay(stdscr, FALSE);

	if (ch2 == ERR) {
		/* ESC 単体 */
		return '\033';
	}
	if (ch2 != '[') {
		/* ESC + 非 '[': 読み捨て */
		return MK_NONE;
	}

	/* CSI シーケンス: 最終バイトを取得 */
	nodelay(stdscr, TRUE);
	int ch3 = getch();
	nodelay(stdscr, FALSE);

	if (ch3 == ERR)
		return MK_NONE;

	switch (ch3) {
	case 'A':
		return MK_UP;
	case 'B':
		return MK_DOWN;
	default:
		return MK_NONE;
	}
}

/*
 * has_save_file
 * セーブファイルが存在するかチェックする
 */
static bool
has_save_file(void)
{
	if (save_file == NULL || *save_file == '\0')
		return false;
	return (access(save_file, F_OK) == 0);
}

/*
 * clear_msg_row
 * フッターメッセージ行をクリアする
 */
static void
clear_msg_row(void)
{
	move(MSG_ROW, 0);
	clrtoeol();
}

/*
 * show_footer_msg
 * フッターメッセージ行にメッセージを中央寄せで表示する
 */
static void
show_footer_msg(int msgno, int color_pair)
{
	int w, col;

	clear_msg_row();
#if defined(COLOR)
	attrset(COLOR_PAIR(color_pair));
#endif
	w = utf8_display_width(mesg[msgno]);
	col = (ROGUE_COLUMNS - w) / 2;
	mvaddstr(MSG_ROW, col, mesg[msgno]);
#if defined(COLOR)
	attrset(COLOR_PAIR(WHITE));
#endif
}

/*
 * draw_item
 * メニュー項目を描画する
 */
static void
draw_item(int idx, bool selected)
{
	int row = item_row[idx];
	int col;
	char buf[MAX_MESG_BUFFER_SIZE];

	snprintf(buf, sizeof(buf), "%s%s", item_prefix[idx],
	    mesg[item_msg[idx]]);

	/* 中央寄せ: 表示幅を計算してセンタリング */
	int width = utf8_display_width(buf);
	col = (ROGUE_COLUMNS - width) / 2;

#if defined(COLOR)
	if (selected) {
		attrset(COLOR_PAIR(YELLOW_REVERSE));
	} else {
		attrset(COLOR_PAIR(WHITE));
	}
#else
	if (selected) {
		attron(A_REVERSE);
	}
#endif

	mvaddstr(row, col - 2, "  ");
	addstr(buf);
	addstr("  ");

#if defined(COLOR)
	attrset(COLOR_PAIR(WHITE));
#else
	attroff(A_REVERSE);
#endif
}

/*
 * try_restore
 * セーブファイルの存在を確認し、リストア可能か判定する
 * リストア可能なら true を返す
 */
static bool
try_restore(void)
{
	if (has_save_file())
		return true;

	show_footer_msg(MENU_NO_SAVE, MAGENTA);
	refresh();
	return false;
}

/*
 * show_menu
 * 起動メニューを表示し、選択結果を返す
 */
int
show_menu(void)
{
	int cur = 0; /* 現在のカーソル位置 */
	int ch;
	int i;
	int title_width, title_col;
	int help_width, help_col;
	int invalid_count = 0; /* 無効キー連打カウンタ */

	clear();

	/* タイトル描画 */
#if defined(COLOR)
	attrset(COLOR_PAIR(YELLOW));
#endif
	title_width = utf8_display_width(mesg[MENU_TITLE]);
	title_col = (ROGUE_COLUMNS - title_width) / 2;
	mvaddstr(5, title_col, mesg[MENU_TITLE]);
#if defined(COLOR)
	attrset(COLOR_PAIR(WHITE));
#endif

	/* 全メニュー項目描画 */
	for (i = 0; i < MENU_ITEMS; i++) {
		draw_item(i, (i == cur));
	}

	/* 操作説明 */
#if defined(COLOR)
	attrset(COLOR_PAIR(WHITE));
#endif
	help_width = utf8_display_width(mesg[MENU_HELP]);
	help_col = (ROGUE_COLUMNS - help_width) / 2;
	mvaddstr(20, help_col, mesg[MENU_HELP]);

	refresh();

	/* 入力ループ */
	for (;;) {
		ch = menu_getchar();

		/* 大文字に正規化 */
		if (ch >= 'a' && ch <= 'z')
			ch = ch - 'a' + 'A';

		/* ダイレクトキー判定 */
		for (i = 0; i < MENU_ITEMS; i++) {
			if (ch == item_key[i])
				break;
		}
		if (i < MENU_ITEMS) {
			clear_msg_row();
			invalid_count = 0;
			/* カーソルを移動して選択表示 */
			if (i != cur) {
				draw_item(cur, false);
				cur = i;
				draw_item(cur, true);
			}
			/* リストア選択時はセーブファイル確認 */
			if (i == 1 && !try_restore()) {
				refresh();
				continue;
			}
			refresh();
			return cur;
		}

		switch (ch) {
		case 'K':
		case MK_UP:
			clear_msg_row();
			invalid_count = 0;
			draw_item(cur, false);
			cur = conn_up[cur];
			draw_item(cur, true);
			refresh();
			break;
		case 'J':
		case MK_DOWN:
			clear_msg_row();
			invalid_count = 0;
			draw_item(cur, false);
			cur = conn_down[cur];
			draw_item(cur, true);
			refresh();
			break;
		case ' ':
		case '\r':
		case '\n':
			clear_msg_row();
			/* リストア選択時はセーブファイル確認 */
			if (cur == 1 && !try_restore()) {
				refresh();
				continue;
			}
			return cur;
		case '\033': /* ESC 単体 */
			return MENU_QUIT;
		case MK_NONE:
			break;
		default:
			/* playmenu 風の無効キーメッセージ */
			if (invalid_count == 0) {
				show_footer_msg(MENU_INVALID1, MAGENTA);
			} else {
				show_footer_msg(MENU_INVALID2, MAGENTA);
			}
			invalid_count++;
			refresh();
			break;
		}
	}
}
