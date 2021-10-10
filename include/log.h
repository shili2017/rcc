#ifndef _LOG_H_
#define _LOG_H_

#define LOG_ERROR 0
#define LOG_WARN 1
#define LOG_INFO 2
#define LOG_DEBUG 3
#define LOG_TRACE 4

#define LOG_LEVEL LOG_INFO

#define ANSI_RED 31
#define ANSI_YELLOW 93
#define ANSI_BLUE 34
#define ANSI_GREEN 32
#define ANSI_GREY 90

int panic(const char* fmt, ...);
int log(unsigned level, const char* fmt, ...);

#define error(fmt, ...) \
        log(LOG_ERROR, fmt, ##__VA_ARGS__)
#define warn(fmt, ...) \
        log(LOG_WARN, fmt, ##__VA_ARGS__)
#define info(fmt, ...) \
        log(LOG_INFO, fmt, ##__VA_ARGS__)
#define debug(fmt, ...) \
        log(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define trace(fmt, ...) \
        log(LOG_TRACE, fmt, ##__VA_ARGS__)

#endif  // _LOG_H_