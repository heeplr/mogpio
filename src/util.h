
#ifndef _UTIL_H
#define _UTIL_H


const char *skip_ws(const char *s, const char *end);
void rtrim_ws(char *s);
bool token_eq(const char *s, const char *tok);
size_t append_fmt(char *dst, size_t cap, size_t used, const char *fmt, ...);

bool parse_u32(const char *s, uint32_t *out);
bool parse_u8(const char *s, uint8_t *out);
bool parse_function(const char *s, hal_gpio_function_t *fn);
bool parse_mode(const char *s, hal_gpio_mode_t *mode);
bool parse_bank_pin(const char *s, uint8_t *bank, uint8_t *pin);
bool parse_value01(const char *s, bool *value);


#endif /* _UTIL_H */
