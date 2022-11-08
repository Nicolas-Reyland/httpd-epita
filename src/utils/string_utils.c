#include "string_utils.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/mem.h"

#define MY_GETLINE_BUFF_INCR 4096

static ssize_t my_getline(char **lineptr, FILE *stream);

/*
** NULL-terminated lines array
*/
char **read_lines_from_stream(FILE *stream, size_t *num_lines_ptr)
{
    char **lines = NULL;
    size_t num_lines = 0;

    char *lineptr = NULL;
    size_t lineptr_n;
    (void)my_getline;

    for (ssize_t num_read = 0;
         (num_read = getline(&lineptr, &lineptr_n, stream)) != -1;)
    {
        if (num_read == 0)
            continue;

        lines = realloc(lines, sizeof(char *) * (num_lines + 1));
        // overwrite '\n' with the end-of-string
        if (lineptr[num_read - 1] == '\n')
            lineptr[num_read - 1] = 0;
        lines[num_lines++] = lineptr;
        lineptr = NULL;
    }
    free(lineptr);
    if (!feof(stream) || ferror(stream))
    {
        // an error occured while reading from the stream
        for (size_t i = 0; i < num_lines; ++i)
            free(lines[i]);
        free(lines);
        // set number of lines to incompatible value with NULL
        *num_lines_ptr = 1;
        return NULL;
    }
    if (num_lines == 0)
    {
        // nothing has been read
        *num_lines_ptr = 0;
        free(lines);
        return NULL;
    }

    // NULL-terminate lines array
    lines = realloc(lines, (num_lines + 1) * sizeof(char **));
    lines[num_lines] = NULL;

    *num_lines_ptr = num_lines;
    return lines;
}

ssize_t my_getline(char **lineptr, FILE *stream)
{
    if (lineptr == NULL)
        return -1;

    *lineptr = malloc(MY_GETLINE_BUFF_INCR);
    if (*lineptr == NULL)
        return -1;

    ssize_t size = 0;
    ssize_t capacity = MY_GETLINE_BUFF_INCR;

    for (char c = 0; (c = fgetc(stream)) != EOF;)
    {
        if (c == '\n')
        {
            *lineptr = realloc(*lineptr, size + 1);
            (*lineptr)[size] = 0;
            return size;
        }
        if (size + 1 == capacity)
            *lineptr = realloc(*lineptr, (capacity += MY_GETLINE_BUFF_INCR));
        (*lineptr)[size++] = c;
    }
    if (size == 0)
    {
        FREE_SET_NULL(*lineptr);
        return -1;
    }

    *lineptr = realloc(*lineptr, size + 1);
    (*lineptr)[size] = 0;
    return size;
}

void skip_all_classifier(char **content, int (*classifier)(int))
{
    for (; **content != 0 && (*classifier)(**content); ++(*content))
        continue;
}

int is_empty_line(char *line)
{
    skip_to_nonwhitespace(&line);
    return *line == 0 || *line == '#';
}

char *token_from_class(char **content, int (*classifier)(int),
                       size_t *token_len)
{
    char *content_start = *content;
    size_t num_chars;
    if (classifier != NULL)
    {
        for (; **content != 0 && (*classifier)(**content); ++(*content))
            continue;
        num_chars = *content - content_start;
    }
    else
    {
        num_chars = strlen(*content);
        *content += num_chars;
    }

    if (token_len != NULL)
        *token_len = num_chars;

    // empty token (null)
    if (num_chars == 0)
        return NULL;

    // create token
    char *token = malloc(num_chars + 1);
    memcpy(token, content_start, num_chars);
    token[num_chars] = 0;

    return token;
}

void skip_to_nonwhitespace(char **content)
{
    // go to the next non-whitespace character
    for (; **content != 0 && isspace(**content); ++(*content))
        continue;
}

int line_is_empty(char *line)
{
    if (line == NULL)
        return 0;

    skip_to_nonwhitespace(&line);
    return *line == 0;
}

int replace_substring(char **str, char *str_start, char *substr,
                      char *replacement)
{
    // replace CAN be null
    if (str == NULL || *str == NULL || substr == NULL)
        return 0;

    size_t str_len = strlen(*str);
    size_t substr_len = strlen(substr);
    size_t replacement_len = replacement == NULL ? 0 : strlen(replacement);
    int mem_shift = replacement_len - substr_len;
    // mem_shift > 0 ?
    bool bigger_replacement = replacement_len > substr_len;

    char *location;
    if ((location = strstr(str_start, substr)) != NULL)
    {
        size_t offset_to_location = location - *str;
        if (bigger_replacement)
        {
            *str = realloc(*str, str_len + mem_shift + 1);
            (*str)[str_len + mem_shift] = 0;
            location = *str + offset_to_location;
        }

        memmove(location + replacement_len, location + substr_len,
                str_len - (offset_to_location + substr_len) + 1);
        if (replacement != NULL)
            memcpy(location, replacement, replacement_len);
        str_len += mem_shift;

        if (!bigger_replacement)
        {
            *str = realloc(*str, str_len + 1);
            (*str)[str_len] = 0;
        }

        return mem_shift;
    }

    return 0;
}
