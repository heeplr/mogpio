
#ifndef _LOGGER_H
#define _LOGGER_H

/* message logging macros */
#ifdef DEBUG_OUT
#define HAVE_LOGGING 1
#define LOGGER($msg, ...) printf( $msg "\n", ##__VA_ARGS__)
#define DLOGGER($level, $msg, ...) printf("%s:%d %s() %s: " $msg "\n", __FILE__, __LINE__, __func__, $level, ##__VA_ARGS__)
#define DEBUG($msg, ...) DLOGGER("DEBUG", $msg, ##__VA_ARGS__)
#define INFO($msg, ...) LOGGER($msg, ##__VA_ARGS__)
#define WARN($msg, ...) LOGGER("WARNING: " $msg, ##__VA_ARGS__)
#define ERROR($msg, ...) DLOGGER("ERROR", $msg, ##__VA_ARGS__)
#else
#define LOGGER(...)
#define DEBUG(...)
#define INFO(...)
#define WARN(...)
#define ERROR(...)
#endif /* DEBUG_OUT */


#ifdef HAVE_LOGGING
#include "pico/stdlib.h"
#endif

#include "bsp/board.h"


#endif /* _LOGGER_H */
