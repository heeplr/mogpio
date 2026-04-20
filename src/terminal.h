
#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void terminal_init(void);
void terminal_task(void);
int terminal_write(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* TERMINAL_H */
