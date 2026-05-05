/*
 * sixel.h
 * sixel グラフィックでモンスターを描画する機能の公開 API
 */

#ifndef ROGUE_SIXEL_H
#define ROGUE_SIXEL_H

#include <stdbool.h>

bool sixel_init(void);
void sixel_shutdown(void);
bool sixel_supported(void);
bool sixel_has_sprite(int ch);
void sixel_mark_cell(int y, int x, int ch);
void sixel_clear_cell(int y, int x);
void sixel_invalidate_all(void);
void sixel_overlay_flush(void);

#endif /* ROGUE_SIXEL_H */
