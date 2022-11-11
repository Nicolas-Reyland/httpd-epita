#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>

#define LOG_STDOUT 000010
#define LOG_STDERR 000100
#define LOG_EXIT 001000

#define LOG_LEVEL_MASK 07

enum log_level
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_EPITA,
};

void log_message(int flags, const char *format, ...);

void log_error(const char *format, ...);

void log_error_and_exit(const char *format, ...);

void log_server(const char *format, ...);

#endif /* !LOGGING_H */
