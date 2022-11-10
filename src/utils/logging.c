#include "logging.h"

#include <stdio.h>
#include <stdlib.h>

enum log_level LOG_LEVEL = LOG_DEBUG;

static void vlog_message(int flags, const char *format, va_list args);

void log_message(int flags, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vlog_message(flags, format, args);

    va_end(args);
}

void log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vlog_message(LOG_ERROR | LOG_STDERR, format, args);

    va_end(args);
}

void log_error_and_exit(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vlog_message(LOG_ERROR | LOG_STDERR | LOG_EXIT, format, args);

    va_end(args);
}

void vlog_message(int flags, const char *format, va_list args)
{
    enum log_level msg_level = flags & LOG_LEVEL_MASK;
    if (msg_level < LOG_LEVEL)
        return;

    if (flags & LOG_STDOUT)
    {
        vfprintf(stdout, format, args);
    }
    else
        flags |= LOG_STDERR;
    if (flags & LOG_STDERR)
    {
        vfprintf(stderr, format, args);
    }
    if (flags & LOG_EXIT)
    {
        va_end(args);
        exit(EXIT_FAILURE);
    }
}
