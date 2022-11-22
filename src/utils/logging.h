#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>

#define LOG_STDOUT 00100
#define LOG_STDERR 01000
#define LOG_EXIT 0010000

#define LOG_LEVEL_MASK 07

enum log_level
{
    LOG_DEBUG = 0,
    LOG_INFO = 01,
    LOG_WARN = 02,
    LOG_ERROR = 04,
    LOG_EPITA = 010,
};

void log_debug(const char *format, ...);

void log_info(const char *format, ...);

void log_warn(const char *format, ...);

void log_error(const char *format, ...);

void log_server(const char *format, ...);

#endif /* !LOGGING_H */
