/*
 * sixel.c
 * sixel グラフィックでモンスターを描画する機能の実装
 *
 * SIXEL マクロが定義されているときのみ実体を持つ。未定義時はすべて no-op。
 */

#include "sixel.h"

#if defined(SIXEL)

#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "utf8.h"

#include "monster_pixmap.h"
#include "rogue.h"

/* 端末セルピクセルサイズ取得失敗時のフォールバック */
#define FALLBACK_CELL_W 8
#define FALLBACK_CELL_H 16

#define ENCODED_PIXMAP_SIZE 128
static bool sixel_supported_flag = false;
static int cell_w = FALLBACK_CELL_W;
static int cell_h = FALLBACK_CELL_H;
static char *encoded_pixmap[ENCODED_PIXMAP_SIZE]; /* index = ASCII char */
static const struct sprite_def *sprite_of[ENCODED_PIXMAP_SIZE];

/* スプライト (地形タイル以外) をセルへ収める方法。
 * 既定は MONSTER_FIT_ASPECT。ドット絵は 16x32 で端末セルと同じ縦横比 1:2 に
 * してあるため、一般的なフォント (8x16, 10x20 など) ならちょうどセル全面に
 * 収まり、ドットは正方形のまま地形タイルと同じ大きさで揃う。
 * セルがそれより縦長な端末では上に余白が入るので、その場合は環境変数
 * ROGUE_SIXEL_FILL=1 で全面に引き伸ばす描き方も選べる。 */
static int sprite_fit_mode = MONSTER_FIT_ASPECT;

/* 直前に sixel を送出したセル状態。
 * ch が 0 ならそのセルは sixel を出していない、
 * 0 以外なら 'A'-'Z' の sixel が描画済み。 */
struct cell_state {
	int ch;
};
static struct cell_state cells[ROGUE_LINES][ROGUE_COLUMNS];
static bool force_redraw_all = false;

/*
 * read_with_timeout
 * stdin から最大 maxlen バイトを timeout_ms ミリ秒以内に読む。
 * 返り値は読み取りバイト数 (0 = タイムアウト, <0 = エラー)
 */
static int
read_with_timeout(char *buf, int maxlen, int timeout_ms)
{
	fd_set rfds;
	struct timeval tv;
	int total = 0;

	while (total < maxlen) {
		FD_ZERO(&rfds);
		FD_SET(STDIN_FILENO, &rfds);
		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;

		int rv = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
		if (rv <= 0) {
			break;
		}
		ssize_t n = read(STDIN_FILENO, buf + total, maxlen - total);
		if (n <= 0)
			break;
		total += (int)n;
		/* ターミネータ 'c' か 't' を見つけたら抜ける */
		if (buf[total - 1] == 'c' || buf[total - 1] == 't')
			break;
		/* 後続バイトのために短いタイムアウトに */
		timeout_ms = 50;
	}
	return total;
}

/*
 * detect_terminal
 * DA1 を送って sixel 対応か、cell pixel size を取得する。
 * stdin/stdout の元属性は呼び出し元で保存・復元する前提。
 */
static bool
detect_terminal(void)
{
	char buf[128];
	int n;
	bool sixel = false;

	/* DA1 送信 */
	if (write(STDOUT_FILENO, "\x1b[c", 3) != 3)
		return false;
	n = read_with_timeout(buf, (int)sizeof(buf) - 1, 200);
	if (n <= 0)
		return false;
	buf[n] = '\0';
	/* レスポンスは ESC [ ? <params> c の形。<params> は ; 区切り。 */
	/* "4" がパラメータ要素として含まれているかを単純に走査 */
	{
		const char *p = buf;
		while (*p) {
			if ((p == buf || p[-1] == '?' || p[-1] == ';') &&
			    p[0] == '4' &&
			    (p[1] == ';' || p[1] == 'c' || p[1] == '\0')) {
				sixel = true;
				break;
			}
			p++;
		}
	}
	if (!sixel)
		return false;

	/* セルピクセルサイズ取得: CSI 16 t -> CSI 6 ; H ; W t */
	if (write(STDOUT_FILENO, "\x1b[16t", 5) != 5)
		return true; /* sixel 自体は対応。サイズはフォールバック値で */
	n = read_with_timeout(buf, (int)sizeof(buf) - 1, 200);
	if (n > 0) {
		buf[n] = '\0';
		const char *p = strstr(buf, "\x1b[6;");
		if (p) {
			int h = 0, w = 0;
			if (sscanf(p, "\x1b[6;%d;%dt", &h, &w) == 2) {
				if (h > 0 && h < 100)
					cell_h = h;
				if (w > 0 && w < 100)
					cell_w = w;
			}
		}
	}
	return true;
}

/*
 * append_str
 * 動的に伸びるバッファに文字列を追記する。失敗時は false。
 */
static bool
append_str(char **bufp, size_t *lenp, size_t *capp, const char *s)
{
	size_t slen = strlen(s);
	if (*lenp + slen + 1 > *capp) {
		size_t newcap = (*capp == 0) ? 256 : *capp * 2;
		while (newcap < *lenp + slen + 1)
			newcap *= 2;
		char *p = realloc(*bufp, newcap);
		if (!p)
			return false;
		*bufp = p;
		*capp = newcap;
	}
	memcpy(*bufp + *lenp, s, slen);
	*lenp += slen;
	(*bufp)[*lenp] = '\0';
	return true;
}

/*
 * encode_pixmap
 * スプライトの文字列ドット絵を cell_w x cell_h ピクセルへ収め、
 * sixel 文字列にエンコードして返す。malloc 済み文字列。失敗時 NULL。
 *
 * 収め方は monster_pixmap_fit() が担当する。縦横比を保ったままセルの中央に
 * 置くので、モンスターの 16x16 は正方形フォントの端末なら原寸、一般的な
 * 8x16 のセルなら 8x8 に縮小されて上下に余白が入る。地形・アイテムの 8x16
 * は 8x16 のセルにちょうど収まる。
 *
 * 透明色 (index 0) のピクセルは出力しない。DCS パラメータ P2=1 を指定し、
 * 未指定ピクセルを「変更なし (transparent)」として残すモードにする。
 * 結果として ncurses が描いた背景文字 (床 '.', 通路 '#' 等) が透けて見える。
 */
static char *
encode_pixmap(const struct sprite_def *sd)
{
	/* 表示サイズはターミナル 1 セル分ピッタリ (cell_w x cell_h)。
	 * 隣のセルにははみ出さない。 */
	const int sw = cell_w;
	const int sh = cell_h;
	char *out = NULL;
	size_t len = 0, cap = 0;
	char tmp[64];
	uint8_t src[MONSTER_PIXMAP_MAX_W * MONSTER_PIXMAP_MAX_H];
	bool used[MONSTER_PALETTE_SIZE];
	uint8_t *scaled;
	int x, y, i;

	if (sw <= 0 || sh <= 0)
		return NULL;

	scaled = malloc((size_t)sw * (size_t)sh);
	if (!scaled)
		return NULL;

	monster_pixmap_decode(sd, src);
	monster_pixmap_fit(src, sd->w, sd->h, scaled, sw, sh,
	    sd->bg ? MONSTER_FIT_FILL : sprite_fit_mode);

	/* 実際に使っている色だけをパレットに登録する。全 64 色を毎セル送ると
	 * 出力量が数倍になるため。 */
	for (i = 0; i < MONSTER_PALETTE_SIZE; i++)
		used[i] = false;
	for (i = 0; i < sw * sh; i++)
		used[scaled[i]] = true;

	/* DCS ヘッダ + ラスター属性
	 * P1=0: ピクセルアスペクト比 default
	 * P2=1: 未指定ピクセルは透明 (既存ピクセル保持)
	 * P3=0: 水平グリッドサイズ default
	 */
	snprintf(tmp, sizeof(tmp), "\x1bP0;1;0q\"1;1;%d;%d", sw, sh);
	if (!append_str(&out, &len, &cap, tmp))
		goto fail;

	/* パレット定義 (index 0 は透明扱いなので出力しない) */
	for (i = 1; i < MONSTER_PALETTE_SIZE; i++) {
		if (!used[i])
			continue;
		snprintf(tmp, sizeof(tmp), "#%d;2;%d;%d;%d", i,
		    monster_palette_rgb[i][0],
		    monster_palette_rgb[i][1],
		    monster_palette_rgb[i][2]);
		if (!append_str(&out, &len, &cap, tmp))
			goto fail;
	}

	/* 6 行ごとのストリップを処理 (index 0 は透明なのでスキップ) */
	for (int strip = 0; strip < sh; strip += 6) {
		int strip_h = (sh - strip < 6) ? (sh - strip) : 6;
		for (int color = 1; color < MONSTER_PALETTE_SIZE; color++) {
			/* この色がストリップ内にあるか確認 */
			bool in_strip = false;
			if (!used[color])
				continue;
			for (y = 0; y < strip_h && !in_strip; y++)
				for (x = 0; x < sw && !in_strip; x++)
					if (scaled[(strip + y) * sw + x] == color)
						in_strip = true;
			if (!in_strip)
				continue;
			snprintf(tmp, sizeof(tmp), "#%d", color);
			if (!append_str(&out, &len, &cap, tmp))
				goto fail;
			/* 各列について sixel 1 文字を出力 (RLE 付き) */
			int run_count = 0;
			char run_ch = 0;
			for (x = 0; x < sw; x++) {
				uint8_t bits = 0;
				for (y = 0; y < strip_h; y++) {
					if (scaled[(strip + y) * sw + x] ==
					    color)
						bits |= (uint8_t)(1 << y);
				}
				char ch = (char)('?' + bits);
				if (run_count == 0) {
					run_ch = ch;
					run_count = 1;
				} else if (ch == run_ch) {
					run_count++;
				} else {
					if (run_count >= 4) {
						snprintf(tmp, sizeof(tmp),
						    "!%d%c", run_count, run_ch);
					} else {
						/* 短い RLE はそのまま並べる */
						int k;
						tmp[0] = '\0';
						for (k = 0; k < run_count &&
						    (size_t)k < sizeof(tmp) - 1;
						    k++)
							tmp[k] = run_ch;
						tmp[k] = '\0';
					}
					if (!append_str(&out, &len, &cap, tmp))
						goto fail;
					run_ch = ch;
					run_count = 1;
				}
			}
			if (run_count > 0) {
				if (run_count >= 4) {
					snprintf(tmp, sizeof(tmp), "!%d%c",
					    run_count, run_ch);
				} else {
					int k;
					tmp[0] = '\0';
					for (k = 0; k < run_count &&
					    (size_t)k < sizeof(tmp) - 1;
					    k++)
						tmp[k] = run_ch;
					tmp[k] = '\0';
				}
				if (!append_str(&out, &len, &cap, tmp))
					goto fail;
			}
			/* 同じストリップ内で次の色のため復帰 */
			if (!append_str(&out, &len, &cap, "$"))
				goto fail;
		}
		/* ストリップを進める */
		if (!append_str(&out, &len, &cap, "-"))
			goto fail;
	}

	/* DCS 終端 ST */
	if (!append_str(&out, &len, &cap, "\x1b\\"))
		goto fail;

	free(scaled);
	return out;

fail:
	free(scaled);
	free(out);
	return NULL;
}

bool
sixel_init(void)
{
	struct termios oldattr, raw;
	bool ok;

	if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
		return false;

	/* stdin を一時的に raw に */
	if (tcgetattr(STDIN_FILENO, &oldattr) != 0)
		return false;
	raw = oldattr;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
		return false;

	ok = detect_terminal();

	{
		const char *fill = getenv("ROGUE_SIXEL_FILL");
		if (fill && *fill && *fill != '0')
			sprite_fit_mode = MONSTER_FIT_FILL;
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);

	if (!ok) {
		sixel_supported_flag = false;
		return false;
	}

	/* sprite_defs の全 sprite をエンコード */
	for (int i = 0; i < ENCODED_PIXMAP_SIZE; i++) {
		encoded_pixmap[i] = NULL;
		sprite_of[i] = NULL;
	}
	for (int i = 0; sprite_defs[i].art; i++) {
		int ch = sprite_defs[i].ch;
		if (ch >= 0 && ch < ENCODED_PIXMAP_SIZE) {
			encoded_pixmap[ch] = encode_pixmap(&sprite_defs[i]);
			sprite_of[ch] = &sprite_defs[i];
		}
	}

	memset(cells, 0, sizeof(cells));
	sixel_supported_flag = true;
	return true;
}

void
sixel_shutdown(void)
{
	for (int i = 0; i < ENCODED_PIXMAP_SIZE; i++) {
		free(encoded_pixmap[i]);
		encoded_pixmap[i] = NULL;
		sprite_of[i] = NULL;
	}
	sixel_supported_flag = false;
}

bool
sixel_supported(void)
{
	return sixel_supported_flag;
}

/*
 * sixel_has_sprite
 * その文字に対応するスプライトが用意されているか。
 * display.c が「ncurses 側では描かない文字」を判定するのに使う。
 */
bool
sixel_has_sprite(int ch)
{
	if (!sixel_supported_flag)
		return false;
	if (ch < 0 || ch >= ENCODED_PIXMAP_SIZE)
		return false;
	return encoded_pixmap[ch] != NULL;
}

/* mvaddch_rogue 経由のヒント。実際の判定は flush 時に mvinch で行う。 */
void
sixel_mark_cell(int y, int x, int ch)
{
	(void)y;
	(void)x;
	(void)ch;
}

void
sixel_clear_cell(int y, int x)
{
	(void)y;
	(void)x;
}

void
sixel_invalidate_all(void)
{
	if (!sixel_supported_flag)
		return;
	force_redraw_all = true;
}

/*
 * terrain_char_at
 * dungeon[][] からそのセルの地形を表す文字を返す。該当なしなら 0。
 * dungeon の添字は画面座標そのもの (room.c の get_dungeon_char と同じ)。
 */
static int
terrain_char_at(int y, int x)
{
	unsigned short mask = dungeon[y][x];

	if (mask & TUNNEL)
		return '#';
	if (mask & DOOR)
		return '+';
	if (mask & HORWALL)
		return '-';
	if (mask & VERTWALL)
		return '|';
	if (mask & FLOOR)
		return '.';
	return 0;
}

/*
 * sixel_overlay_flush
 * 画面全セルを ncurses バッファから mvinch で読み出し、スプライトを持つ
 * 文字を見つけたら sixel を **毎回無条件に** 上書き送出する。
 *
 * 描画は 2 層。モンスターやアイテムのスプライトは透明部分を持つので、
 * そのままだと ncurses が描いた文字 ('B' など) が透けて見えてしまう。
 * そこで先に地形タイル (床・通路・扉…) でセル全面を不透明に塗り、その上に
 * スプライトを重ねる。地形タイル自身 (sd->bg) は 1 層で足りる。
 *
 * 前回状態のキャッシュ比較は行わない。これにより ncurses が「変化なし」
 * と判断してスキップしたセルや、メッセージ表示等の副作用で物理 sixel が
 * 消えたケースでも、次の refresh で確実に復元される。
 *
 * モンスターが画面から消えた (床文字に置き換わった) 場合は、ncurses 側で
 * セル領域がそのまま文字で再描画されるため sixel は自動的に消える。
 */
void
sixel_overlay_flush(void)
{
	if (!sixel_supported_flag)
		return;

	bool any = false;
	int y, x;

	(void)force_redraw_all; /* 廃止 */

	/* row 0 はメッセージ行、row ROGUE_LINES-1 はステータス行で
	 * 日本語 wide char が含まれる。mvinch がそれらに対し正しい値を
	 * 返さないことがあるので、モンスターが出現する row 1..22 のみ
	 * スキャンする。 */
	for (y = MIN_ROW; y < ROGUE_LINES - 1; y++) {
		for (x = 0; x < ROGUE_COLUMNS; x++) {
			chtype phys = mvinch(y, x) & A_CHARTEXT;
			int ch = (int)phys;
			const struct sprite_def *sd;

			if (ch < 0x20 || ch >= ENCODED_PIXMAP_SIZE) {
				cells[y][x].ch = 0;
				continue;
			}
			sd = sprite_of[ch];
			if (!sd || !encoded_pixmap[ch]) {
				cells[y][x].ch = 0;
				continue;
			}

			if (!sd->bg) {
				/* 下敷きの地形タイル。分からなければ床にする。 */
				int bg = terrain_char_at(y, x);
				if (bg == 0 || !encoded_pixmap[bg])
					bg = '.';
				if (encoded_pixmap[bg]) {
					fprintf(stdout, "\x1b[%d;%dH",
					    y + 1, x + 1);
					fputs(encoded_pixmap[bg], stdout);
				}
			}
			fprintf(stdout, "\x1b[%d;%dH", y + 1, x + 1);
			fputs(encoded_pixmap[ch], stdout);
			cells[y][x].ch = ch;
			any = true;
		}
	}
	if (any)
		fflush(stdout);
}

#else /* not SIXEL */

bool
sixel_init(void)
{
	return false;
}

void
sixel_shutdown(void)
{
}

bool
sixel_supported(void)
{
	return false;
}

bool
sixel_has_sprite(int ch)
{
	(void)ch;
	return false;
}

void
sixel_mark_cell(int y, int x, int ch)
{
	(void)y;
	(void)x;
	(void)ch;
}

void
sixel_clear_cell(int y, int x)
{
	(void)y;
	(void)x;
}

void
sixel_invalidate_all(void)
{
}

void
sixel_overlay_flush(void)
{
}

#endif /* SIXEL */
