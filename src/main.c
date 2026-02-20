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
#include <string.h>

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
			snprintf(buf, sizeof(buf), mesg[10], nick_name);
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
 * skip_ws
 * 空白文字をスキップする
 */
static const char *
skip_ws(const char *p)
{
	while (*p == ' ' || *p == '\t')
		p++;
	return p;
}

/*
 * parse_quoted_string
 * "..." 形式の文字列を解析し、エスケープ処理を行う
 * 返り値: 成功時は消費バイト数、失敗時は -1
 */
static int
parse_quoted_string(const char *p, char *out, int maxlen)
{
	const char *start = p;
	int i = 0;

	if (*p != '"')
		return -1;
	p++;

	while (*p && *p != '"') {
		if (*p == '\\') {
			p++;
			switch (*p) {
			case '"':
				if (i < maxlen - 1)
					out[i++] = '"';
				break;
			case '\\':
				if (i < maxlen - 1)
					out[i++] = '\\';
				break;
			case 'n':
				if (i < maxlen - 1)
					out[i++] = '\n';
				break;
			case 't':
				if (i < maxlen - 1)
					out[i++] = '\t';
				break;
			case '\0':
				goto err;
			default:
				if (i < maxlen - 1)
					out[i++] = *p;
				break;
			}
		} else {
			if (i < maxlen - 1)
				out[i++] = *p;
		}
		p++;
	}

	if (*p != '"')
		goto err;

	out[i] = '\0';
	return (int)(p + 1 - start);
err:
	return -1;
}

/*
 * read_mesg
 * メッセージファイル読み込み関数 (TOML サブセット形式)
 * 返り値: 成功:0, 失敗:1
 */
int
read_mesg(char *argv_msgfile)
{
	FILE *mesg_file;
	utf8_int8_t buf[MAX_MESG_LINE_SIZE];

	if ((mesg_file = fopen(argv_msgfile, "r")) == NULL) {
		fprintf(stderr, "Cannot open message file '%s'\n",
		    argv_msgfile);
		return 1;
	}

	umesg[0].string[0] = '\0';
	umesg[0].size = 0;

	while (fgets(buf, sizeof(buf), mesg_file) != NULL) {
		const char *p;
		char *nl;
		int n;
		size_t buflen;

		/* 行が長すぎる場合は残りを読み飛ばす */
		buflen = strlen(buf);
		if (buflen == sizeof(buf) - 1 && buf[sizeof(buf) - 2] != '\n') {
			fprintf(stderr, "Line too long in '%s', skipping\n",
			    argv_msgfile);
			int c;
			while ((c = fgetc(mesg_file)) != '\n' && c != EOF)
				;
			continue;
		}

		/* 末尾の改行/CRを除去 */
		nl = buf + buflen;
		while (nl > buf && (nl[-1] == '\n' || nl[-1] == '\r'))
			*--nl = '\0';

		/* 先頭の空白をスキップ */
		p = skip_ws(buf);

		/* 空行・コメント・セクションヘッダはスキップ */
		if (*p == '\0' || *p == '#' || *p == '[')
			continue;

		/* 番号をパース */
		if (*p < '0' || *p > '9') {
			fprintf(stderr, "Illegal format in '%s': %s\n",
			    argv_msgfile, buf);
			fclose(mesg_file);
			return 1;
		}

		n = 0;
		while (*p >= '0' && *p <= '9') {
			if (n > MESSAGE_QUANTITY)
				break;
			n = n * 10 + (*p++ - '0');
		}

		if (n <= 0 || n > MESSAGE_QUANTITY) {
			fprintf(stderr,
			    "Message number %d out of range in '%s'\n", n,
			    argv_msgfile);
			fclose(mesg_file);
			return 1;
		}

		/* '=' を確認 */
		p = skip_ws(p);
		if (*p != '=') {
			fprintf(stderr, "Expected '=' in '%s': %s\n",
			    argv_msgfile, buf);
			fclose(mesg_file);
			return 1;
		}
		p++;
		p = skip_ws(p);

		/* "..." を解析 */
		if (parse_quoted_string(p, mesg[n], MAX_MESG_BUFFER_SIZE) < 0) {
			fprintf(stderr, "Illegal string in '%s': %s\n",
			    argv_msgfile, buf);
			fclose(mesg_file);
			return 1;
		}

		strncpy(umesg[n].string, mesg[n], MAX_MESG_BUFFER_SIZE - 1);
		umesg[n].string[MAX_MESG_BUFFER_SIZE - 1] = '\0';
		umesg[n].size = utf8size(umesg[n].string);
	}

	fclose(mesg_file);
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
