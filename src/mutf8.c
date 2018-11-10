#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "mutf8.h"

/**
 * @private
 * @var uint8_t m_utf8_jump_table[256]
 * @author Atsushi Enomoto  <atsushi@ximian.com>
 * @sa https://github.com/corngood/mono/blob/master/eglib/src/gutf8.c
 * 
 * @note
 *  base var is "const guchar g_utf8_jump_table[256]".
 */
/*
 * Index into the table below with the first byte of a UTF-8 sequence to get
 * the number of bytes that are supposed to follow it to complete the sequence.
 *
 * Note that *legal* UTF-8 values can't have 5-bytes or 6-bytes. The table is left
 * as-is for anyone who may want to do such conversion, which was allowed in
 * earlier algorithms.
*/
static const uint8_t m_utf8_jump_table[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1};

/**
 * @public
 * @fn uint8_t m_utf8_ch_byte_size(const m_char8_t *character)
 * @brief utf8 character byte size
 * @param[in] character - utf8 character
 * @return utf8 character byte size
 * @note
 *   do not use null-terminated string.
 *   that *legal* UTF-8 values can't have 5-bytes or 6-bytes.
 */
uint8_t m_utf8_ch_byte_size(const m_char8_t *character)
{
    return m_utf8_jump_table[(uint8_t)(*character)];
}

/**
 * @public
 * @fn bool m_utf8_ch_validate(const m_char8_t *character, size_t character_bytesize)
 * @brief utf8 character validate
 * @param[in] character - utf8 character
 * @param[in] character_bytesize - utf8 character byte size
 * @return true valid, false invalid
 * @author Atsushi Enomoto  <atsushi@ximian.com>
 * @sa https://github.com/corngood/mono/blob/master/eglib/src/gutf8.c
 * @note
 *   do not use null-terminated string.
 *   base function is "static gboolean utf8_validate (const unsigned char *inptr, size_t len)".
 */
bool m_utf8_ch_validate(const m_char8_t *character, size_t character_size)
{
    uint8_t *inptr = (uint8_t *)character;
    const uint8_t *ptr = inptr + character_size;
    uint8_t c;

    /* Everything falls through when TRUE... */
    switch (character_size)
    {
    default:
        return false;
    case 4:
        if ((c = (*--ptr)) < 0x80 || c > 0xBF)
            return false;

        if ((c == 0xBF || c == 0xBE) && ptr[-1] == 0xBF)
        {
            if (ptr[-2] == 0x8F || ptr[-2] == 0x9F ||
                ptr[-2] == 0xAF || ptr[-2] == 0xBF)
                return false;
        }
        /* fall through */
    case 3:
        if ((c = (*--ptr)) < 0x80 || c > 0xBF)
            return false;
        /* fall through */
    case 2:
        if ((c = (*--ptr)) < 0x80 || c > 0xBF)
            return false;

        /* no fall-through in this inner switch */
        switch (*inptr)
        {
        case 0xE0:
            if (c < 0xA0)
                return false;
            break;
        case 0xED:
            if (c > 0x9F)
                return false;
            break;
        case 0xEF:
            if (c == 0xB7 && (ptr[1] > 0x8F && ptr[1] < 0xB0))
                return false;
            if (c == 0xBF && (ptr[1] == 0xBE || ptr[1] == 0xBF))
                return false;
            break;
        case 0xF0:
            if (c < 0x90)
                return false;
            break;
        case 0xF4:
            if (c > 0x8F)
                return false;
            break;
        default:
            if (c < 0x80)
                return false;
            break;
        }
    case 1:
        if (*inptr >= 0x80 && *inptr < 0xC2)
            return false;
    }

    if (*inptr > 0xF4)
        return false;

    return true;
}

/**
 * @public
 * @fn uint32_t m_utf8_to_unicode(const m_char8_t *character)
 * @brief utf8 character convert to unicode
 * @param[in] character  - utf8 character
 * @return unicode, and convert to 0 when failed.
 * @author FUNABARA Masao
 */
uint32_t m_utf8_to_unicode(const m_char8_t *character)
{
    uint8_t *inptr = (uint8_t *)character;
    uint8_t character_size = m_utf8_ch_byte_size(character);

    bool result = m_utf8_ch_validate(character, character_size);
    if (result == false)
    {
        return 0;
    }

    uint8_t *ptr = inptr + (character_size - 1);
    uint32_t unicode = 0;
    uint32_t shift_size = 0;
    uint8_t mask = 0x00;
    switch (character_size)
    {
    default:
        return 0;
    case 4:
        mask = 0x3F;
        unicode = (*ptr & mask);
        ptr--;
        /* fall through */
    case 3:
        mask = 0x3F;
        if (character_size == 4)
            shift_size = 6;
        if (character_size == 3)
            shift_size = 0;
        unicode += ((*ptr & mask) << shift_size);
        ptr--;
        /* fall through */
    case 2:
        mask = 0x3F;
        if (character_size == 4)
            shift_size = 12;
        if (character_size == 3)
            shift_size = 6;
        if (character_size == 2)
            shift_size = 0;
        unicode += ((*ptr & mask) << shift_size);
        ptr--;
        /* fall through */
    case 1:
        if (character_size == 4)
        {
            mask = 0x0F;
            shift_size = 18;
        }
        if (character_size == 3)
        {
            mask = 0x1F;
            shift_size = 12;
        }
        if (character_size == 2)
        {
            mask = 0x3F;
            shift_size = 6;
        }
        if (character_size == 1)
        {
            mask = 0xFF;
            shift_size = 0;
        }
        unicode += ((*ptr & mask) << shift_size);
    }
    return unicode;
}

/**
 * @private
 * @var te
 * @author cxxxr
 * @sa https://github.com/cxxxr/uemacs/blob/master/utf8.c
 * @note
 *   base struct is "struct te".
 */
struct te
{
    uint32_t a;
    uint32_t b;
};

// clang-format off
/**
 * @private
 * @var static struct te eastasian_full[]
 * @author cxxxr
 * @sa https://github.com/cxxxr/uemacs/blob/master/utf8.c
 * @note
 *   base var is "te eastasian_full[]".
 */
static struct te eastasian_full[] = {
    {0x01100,0x0115f}, {0x02329,0x0232a}, {0x02e80,0x02e99}, {0x02e9b,0x02ef3},
    {0x02f00,0x02fd5}, {0x02ff0,0x02ffb}, {0x03000,0x0303e}, {0x03041,0x03096},
    {0x03099,0x030ff}, {0x03105,0x0312d}, {0x03131,0x0318e}, {0x03190,0x031ba},
    {0x031c0,0x031e3}, {0x031f0,0x0321e}, {0x03220,0x03247}, {0x03250,0x032fe},
    {0x03300,0x04dbf}, {0x04e00,0x0a48c}, {0x0a490,0x0a4c6}, {0x0a960,0x0a97c},
    {0x0ac00,0x0d7a3}, {0x0f900,0x0faff}, {0x0fe10,0x0fe19}, {0x0fe30,0x0fe52},
    {0x0fe54,0x0fe66}, {0x0fe68,0x0fe6b}, {0x0ff01,0x0ff60}, {0x0ffe0,0x0ffe6},
    {0x1b000,0x1b001}, {0x1f200,0x1f202}, {0x1f210,0x1f23a}, {0x1f240,0x1f248},
    {0x1f250,0x1f251}, {0x20000,0x2fffd}, {0x30000,0x3fffd}
};

/**
 * @private
 * @var struct te eastasian_ambiguous[]
 * @author cxxxr
 * @sa https://github.com/cxxxr/uemacs/blob/master/utf8.c
 * @note
 *   base var is "te eastasian_ambiguous[]".
 */
static struct te eastasian_ambiguous[] = {
    {0x000a1,0x000a1}, {0x000a4,0x000a4}, {0x000a7,0x000a8}, {0x000aa,0x000aa},
    {0x000ad,0x000ae}, {0x000b0,0x000b4}, {0x000b6,0x000ba}, {0x000bc,0x000bf},
    {0x000c6,0x000c6}, {0x000d0,0x000d0}, {0x000d7,0x000d8}, {0x000de,0x000e1},
    {0x000e6,0x000e6}, {0x000e8,0x000ea}, {0x000ec,0x000ed}, {0x000f0,0x000f0},
    {0x000f2,0x000f3}, {0x000f7,0x000fa}, {0x000fc,0x000fc}, {0x000fe,0x000fe},
    {0x00101,0x00101}, {0x00111,0x00111}, {0x00113,0x00113}, {0x0011b,0x0011b},
    {0x00126,0x00127}, {0x0012b,0x0012b}, {0x00131,0x00133}, {0x00138,0x00138},
    {0x0013f,0x00142}, {0x00144,0x00144}, {0x00148,0x0014b}, {0x0014d,0x0014d},
    {0x00152,0x00153}, {0x00166,0x00167}, {0x0016b,0x0016b}, {0x001ce,0x001ce},
    {0x001d0,0x001d0}, {0x001d2,0x001d2}, {0x001d4,0x001d4}, {0x001d6,0x001d6},
    {0x001d8,0x001d8}, {0x001da,0x001da}, {0x001dc,0x001dc}, {0x00251,0x00251},
    {0x00261,0x00261}, {0x002c4,0x002c4}, {0x002c7,0x002c7}, {0x002c9,0x002cb},
    {0x002cd,0x002cd}, {0x002d0,0x002d0}, {0x002d8,0x002db}, {0x002dd,0x002dd},
    {0x002df,0x002df}, {0x00300,0x0036f}, {0x00391,0x003a1}, {0x003a3,0x003a9},
    {0x003b1,0x003c1}, {0x003c3,0x003c9}, {0x00401,0x00401}, {0x00410,0x0044f},
    {0x00451,0x00451}, {0x02010,0x02010}, {0x02013,0x02016}, {0x02018,0x02019},
    {0x0201c,0x0201d}, {0x02020,0x02022}, {0x02024,0x02027}, {0x02030,0x02030},
    {0x02032,0x02033}, {0x02035,0x02035}, {0x0203b,0x0203b}, {0x0203e,0x0203e},
    {0x02074,0x02074}, {0x0207f,0x0207f}, {0x02081,0x02084}, {0x020ac,0x020ac},
    {0x02103,0x02103}, {0x02105,0x02105}, {0x02109,0x02109}, {0x02113,0x02113},
    {0x02116,0x02116}, {0x02121,0x02122}, {0x02126,0x02126}, {0x0212b,0x0212b},
    {0x02153,0x02154}, {0x0215b,0x0215e}, {0x02160,0x0216b}, {0x02170,0x02179},
    {0x02189,0x02189}, {0x02190,0x02199}, {0x021b8,0x021b9}, {0x021d2,0x021d2},
    {0x021d4,0x021d4}, {0x021e7,0x021e7}, {0x02200,0x02200}, {0x02202,0x02203},
    {0x02207,0x02208}, {0x0220b,0x0220b}, {0x0220f,0x0220f}, {0x02211,0x02211},
    {0x02215,0x02215}, {0x0221a,0x0221a}, {0x0221d,0x02220}, {0x02223,0x02223},
    {0x02225,0x02225}, {0x02227,0x0222c}, {0x0222e,0x0222e}, {0x02234,0x02237},
    {0x0223c,0x0223d}, {0x02248,0x02248}, {0x0224c,0x0224c}, {0x02252,0x02252},
    {0x02260,0x02261}, {0x02264,0x02267}, {0x0226a,0x0226b}, {0x0226e,0x0226f},
    {0x02282,0x02283}, {0x02286,0x02287}, {0x02295,0x02295}, {0x02299,0x02299},
    {0x022a5,0x022a5}, {0x022bf,0x022bf}, {0x02312,0x02312}, {0x02460,0x024e9},
    {0x024eb,0x0254b}, {0x02550,0x02573}, {0x02580,0x0258f}, {0x02592,0x02595},
    {0x025a0,0x025a1}, {0x025a3,0x025a9}, {0x025b2,0x025b3}, {0x025b6,0x025b7},
    {0x025bc,0x025bd}, {0x025c0,0x025c1}, {0x025c6,0x025c8}, {0x025cb,0x025cb},
    {0x025ce,0x025d1}, {0x025e2,0x025e5}, {0x025ef,0x025ef}, {0x02605,0x02606},
    {0x02609,0x02609}, {0x0260e,0x0260f}, {0x02614,0x02615}, {0x0261c,0x0261c},
    {0x0261e,0x0261e}, {0x02640,0x02640}, {0x02642,0x02642}, {0x02660,0x02661},
    {0x02663,0x02665}, {0x02667,0x0266a}, {0x0266c,0x0266d}, {0x0266f,0x0266f},
    {0x0269e,0x0269f}, {0x026be,0x026bf}, {0x026c4,0x026cd}, {0x026cf,0x026e1},
    {0x026e3,0x026e3}, {0x026e8,0x026ff}, {0x0273d,0x0273d}, {0x02757,0x02757},
    {0x02776,0x0277f}, {0x02b55,0x02b59}, {0x03248,0x0324f}, {0x0e000,0x0f8ff},
    {0x0fe00,0x0fe0f}, {0x0fffd,0x0fffd}, {0x1f100,0x1f10a}, {0x1f110,0x1f12d},
    {0x1f130,0x1f169}, {0x1f170,0x1f19a}, {0xe0100,0xe01ef}, {0xf0000,0xffffd},
    {0x100000,0x10fffd}
};

/**
 * @private
 * @var static struct te emoji_cjk[]
 * @author FUNABARA Masao
 * @sa http://unicode.org/emoji/charts/emoji-list.html
 */
static struct te emoji_cjk[] = {
    {0x1f004,0x1f004}, {0x1f0cf,0x1f0cf}, {0x1f170,0x1f171}, {0x1f17e,0x1f17e},
    {0x1f17f,0x1f17f}, {0x1f18e,0x1f18e}, {0x1f191,0x1f19a}, {0x1f201,0x1f202},
    {0x1f21a,0x1f21a}, {0x1f22f,0x1f22f}, {0x1f232,0x1f23a}, {0x1f250,0x1f251},
    {0x1f300,0x1f320}, {0x1f321,0x1f321}, {0x1f324,0x1f32c}, {0x1f32d,0x1f32f},
    {0x1f330,0x1f335}, {0x1f336,0x1f336}, {0x1f337,0x1f37c}, {0x1f37d,0x1f37d},
    {0x1f37e,0x1f37f}, {0x1f380,0x1f393}, {0x1f396,0x1f397}, {0x1f399,0x1f39b},
    {0x1f39e,0x1f39f}, {0x1f3a0,0x1f3c4}, {0x1f3c5,0x1f3c5}, {0x1f3c6,0x1f3ca},
    {0x1f3cb,0x1f3ce}, {0x1f3cf,0x1f3d3}, {0x1f3d4,0x1f3df}, {0x1f3e0,0x1f3f0},
    {0x1f3f3,0x1f3f5}, {0x1f3f7,0x1f3f7}, {0x1f3f8,0x1f3ff}, {0x1f400,0x1f43e},
    {0x1f43f,0x1f43f}, {0x1f440,0x1f440}, {0x1f441,0x1f441}, {0x1f442,0x1f4f7},
    {0x1f4f8,0x1f4f8}, {0x1f4f9,0x1f4fc}, {0x1f4fd,0x1f4fd}, {0x1f4ff,0x1f4ff},
    {0x1f500,0x1f53d}, {0x1f549,0x1f54a}, {0x1f54b,0x1f54e}, {0x1f550,0x1f567},
    {0x1f56f,0x1f570}, {0x1f573,0x1f579}, {0x1f57a,0x1f57a}, {0x1f587,0x1f587},
    {0x1f58a,0x1f58d}, {0x1f590,0x1f590}, {0x1f595,0x1f596}, {0x1f5a4,0x1f5a4},
    {0x1f5a5,0x1f5a5}, {0x1f5a8,0x1f5a8}, {0x1f5b1,0x1f5b2}, {0x1f5bc,0x1f5bc},
    {0x1f5c2,0x1f5c4}, {0x1f5d1,0x1f5d3}, {0x1f5dc,0x1f5de}, {0x1f5e1,0x1f5e1},
    {0x1f5e3,0x1f5e3}, {0x1f5e8,0x1f5e8}, {0x1f5ef,0x1f5ef}, {0x1f5f3,0x1f5f3},
    {0x1f5fa,0x1f5fa}, {0x1f5fb,0x1f5ff}, {0x1f600,0x1f600}, {0x1f601,0x1f610},
    {0x1f611,0x1f611}, {0x1f612,0x1f614}, {0x1f615,0x1f615}, {0x1f616,0x1f616},
    {0x1f617,0x1f617}, {0x1f618,0x1f618}, {0x1f619,0x1f619}, {0x1f61a,0x1f61a},
    {0x1f61b,0x1f61b}, {0x1f61c,0x1f61e}, {0x1f61f,0x1f61f}, {0x1f620,0x1f625},
    {0x1f626,0x1f627}, {0x1f628,0x1f62b}, {0x1f62c,0x1f62c}, {0x1f62d,0x1f62d},
    {0x1f62e,0x1f62f}, {0x1f630,0x1f633}, {0x1f634,0x1f634}, {0x1f635,0x1f640},
    {0x1f641,0x1f642}, {0x1f643,0x1f644}, {0x1f645,0x1f64f}, {0x1f680,0x1f6c5},
    {0x1f6cb,0x1f6cf}, {0x1f6d0,0x1f6d0}, {0x1f6d1,0x1f6d2}, {0x1f6e0,0x1f6e5},
    {0x1f6e9,0x1f6e9}, {0x1f6eb,0x1f6ec}, {0x1f6f0,0x1f6f0}, {0x1f6f3,0x1f6f3},
    {0x1f6f4,0x1f6f6}, {0x1f6f7,0x1f6f7}, {0x1f6f9,0x1f6f9}, {0x1f910,0x1f918},
    {0x1f919,0x1f91e}, {0x1f91f,0x1f91f}, {0x1f920,0x1f927}, {0x1f928,0x1f92f},
    {0x1f930,0x1f930}, {0x1f931,0x1f932}, {0x1f933,0x1f93a}, {0x1f93c,0x1f93e},
    {0x1f940,0x1f945}, {0x1f947,0x1f94b}, {0x1f94c,0x1f94c}, {0x1f94d,0x1f94f},
    {0x1f950,0x1f95e}, {0x1f95f,0x1f96b}, {0x1f96c,0x1f970}, {0x1f973,0x1f976},
    {0x1f97a,0x1f97a}, {0x1f97c,0x1f97f}, {0x1f980,0x1f984}, {0x1f985,0x1f991},
    {0x1f992,0x1f997}, {0x1f998,0x1f9a2}, {0x1f9b0,0x1f9b9}, {0x1f9c0,0x1f9c0},
    {0x1f9c1,0x1f9c2}, {0x1f9d0,0x1f9e6}, {0x1f9e7,0x1f9ff}
};
// clang-format on

/**
 * @private
 * @fn static int comp_te(const void *key, const void *elt)
 * @author cxxxr
 * @sa https://github.com/cxxxr/uemacs/blob/master/utf8.c
 * @note
 *   base function is "static int comp_te(const void *key, const void *elt)".
 */
static int comp_te(const void *key, const void *elt)
{
    struct te *te = (struct te *)elt;
    uint32_t *c = (uint32_t *)key;
    if (*c < te->a)
        return -1;
    if (te->a <= *c && *c <= te->b)
        return 0;
    return 1;
}

/**
 * @private
 * @fn static int search_table(struct te *table, int size, uint32_t c)
 * @author cxxxr
 * @sa https://github.com/cxxxr/uemacs/blob/master/utf8.c
 * @note
 *   base function is "static int search_table(struct te *table, int size, unicode_t c)".
 */
static int search_table(struct te *table, int size, uint32_t c)
{
    struct te *res;
    res = bsearch(&c, table, size, sizeof(struct te), comp_te);
    return res != NULL;
}

/**
 * @public
 * @fn size_t m_utf8_display_width(const m_char8_t *character)
 * @author cxxxr
 * @sa https://github.com/cxxxr/uemacs/blob/master/utf8.c
 * @note
 *   base function is "int unicode_width(unicode_t c)".
 */
size_t m_utf8_display_width(const m_char8_t *character)
{
    uint32_t c = m_utf8_to_unicode(character);
    if (search_table(eastasian_full,
                     sizeof(eastasian_full) / sizeof(struct te),
                     c))
        return 2;
    if (search_table(eastasian_ambiguous,
                     sizeof(eastasian_ambiguous) / sizeof(struct te),
                     c))
        return 2;
    if (search_table(emoji_cjk,
                     sizeof(emoji_cjk) / sizeof(struct te),
                     c))
    {
        return 2;
        // if (strcmp(locale, "ja_JP.UTF-8") == 0)
        // {
        //     return 2;
        // }
        // else
        // {
        //     return 2;
        // }
    }
    return 1;
}

/**
 * @public
 * @fn int64_t m_utf8_str_byte_size(const m_char8_t *str, size_t max_str_bytesize)
 * @brief utf8 string byte size
 * @param[in] str - utf8 string
 * @param[in] max_str_bytesize - utf8 string byte size( add null-terminated string size)
 * @return string byte size( add null-terminated string size)
 * @author FUNABARA Masao
 * @note
 *   add null-terminated string size.
 */
int64_t m_utf8_str_byte_size(const m_char8_t *str, size_t max_str_bytesize)
{
    const int64_t max_size = (int64_t)max_str_bytesize;
    int64_t str_size = 0;
    uint8_t size = 0;

    while (*str && str_size < max_size)
    {
        size = m_utf8_ch_byte_size(str);
        str_size += size;
        str += size;
    }

    if (str_size >= max_size)
    {
        str_size -= size;
    }
    if (max_size > 0)
    {
        str_size += 1;
    }
    return str_size;
}

/**
 * @public
 * @fn bool m_utf8_str_validate(const m_char8_t *str, size_t max_str_bytesize)
 * @brief utf8 string validate
 * @param[in] str - utf8 string
 * @param[in] max_str_bytesize - utf8 string byte size( add null-terminated string size)
 * @return true valid, false invalid
 * @author FUNABARA Masao
 * @note
 *   add null-terminated string size.
 */
bool m_utf8_str_validate(const m_char8_t *str, size_t max_str_bytesize)
{
    int64_t max_size = m_utf8_str_byte_size(str, max_str_bytesize);
    int64_t str_size = 0;

    while (*str && str_size < max_size)
    {
        uint8_t ch_byte_size = m_utf8_ch_byte_size(str);
        bool result = m_utf8_ch_validate(str, ch_byte_size);
        if (result == false)
        {
            return false;
        }
        str_size += ch_byte_size;
        str += ch_byte_size;
    }

    if (str_size >= max_size)
    {
        return false;
    }

    return true;
}

/**
 * @public
 * @fn int64_t m_utf8_str_display_count(const m_char8_t *str, size_t max_str_bytesize)
 * @brief utf8 string display character count
 * @param[in] str - utf8 string
 * @param[in] max_str_bytesize - utf8 string byte size( add null-terminated string size)
 * @return count size;
 * @author FUNABARA Masao
 * @note
 *   add null-terminated string size.
 *   Don't correspond Emoji and Combine Strings.
 */
int64_t m_utf8_str_display_count(const m_char8_t *str, size_t max_str_bytesize)
{
    int64_t max_size = m_utf8_str_byte_size(str, max_str_bytesize);
    int64_t str_size = 0;
    int64_t display_count = 0;

    while (*str && str_size < max_size)
    {
        uint8_t ch_byte_size = m_utf8_ch_byte_size(str);
        str_size += ch_byte_size;
        str += ch_byte_size;
        display_count++;
    }

    if (str_size >= max_size)
    {
        return display_count;
    }

    return display_count;
}

/**
 * @public
 * @fn bool m_utf8_str_cpy(m_char8_t *dst, size_t dst_array_size, const m_char8_t *src, size_t src_size)
 * @param[out] dst - array to save string
 * @param[in] dst_array_size - array size
 * @param[in] src - source string
 * @param[in] src_size - source string size
 * @return always true
 */
bool m_utf8_str_cpy(m_char8_t *dst, const size_t dst_array_size, const m_char8_t *src, size_t src_size)
{
    int64_t src_byte_size = m_utf8_str_byte_size(src, src_size);
    uint8_t str_size = 0;

    size_t i = 0;
    while (i < dst_array_size)
    {
        uint8_t size = m_utf8_ch_byte_size(src);
        if (str_size + size >= dst_array_size)
        {
            *dst = '\0';
            break;
        }
        if (str_size + size >= src_byte_size)
        {
            *dst = '\0';
            break;
        }
        for (int j = 0; j < size; j++)
        {
            *dst = *src;
            dst++;
            src++;
            i++;
            str_size++;
        }
    }

    return true;
}

/**
 * @public
 * @fn bool m_utf8_str_cat(m_char8_t *dst, const size_t dst_array_size, const m_char8_t *src, size_t src_size)
 * @param[out] dst - array to save string
 * @param[in] dst_array_size - array size
 * @param[in] src - source string
 * @param[in] src_size - source string size
 * @return always true
 */
bool m_utf8_str_cat(m_char8_t *dst, const size_t dst_array_size, const m_char8_t *src, size_t src_size)
{
    int64_t dst_byte_size = m_utf8_str_byte_size(dst, dst_array_size) - 1;
    m_char8_t *inptr = dst;
    int64_t inptr_size = dst_array_size - dst_byte_size;
    inptr += dst_byte_size;

    m_utf8_str_cpy(inptr, inptr_size, src, src_size);

    return true;
}