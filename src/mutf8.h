#ifndef MUTF8_H
#define MUTF8_H

typedef char m_char8_t;
extern uint8_t m_utf8_ch_byte_size(const m_char8_t *character);
extern bool m_utf8_ch_validate(const m_char8_t *character, size_t character_bytesize);
extern uint32_t m_utf8_to_unicode(const m_char8_t *character);
extern size_t m_utf8_display_width(const m_char8_t *character);
extern int64_t m_utf8_str_byte_size(const m_char8_t *str, size_t max_str_bytesize);
extern bool m_utf8_str_validate(const m_char8_t *str, size_t max_str_bytesize);
extern int64_t m_utf8_str_display_count(const m_char8_t *str, size_t max_str_bytesize);
extern bool m_utf8_str_cpy(m_char8_t *dst, const size_t dst_array_size, const m_char8_t *src, size_t src_size);
extern bool m_utf8_str_cat(m_char8_t *dst, const size_t dst_array_size, const m_char8_t *src, size_t src_size);

#endif /* end MUTF_8 */
