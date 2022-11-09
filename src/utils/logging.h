#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>

#define LOG_STDOUT 00001
#define LOG_STDERR 00010
#define LOG_EXIT 00100

void log_message(int flags, const char *format, ...);

void log_error(const char *format, ...);

void log_error_and_exit(const char *format, ...);

#endif /* !LOGGING_H */
