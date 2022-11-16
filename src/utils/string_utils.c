#include "string_utils.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/mem.h"
#include "utils/mylibc/my_getline.h"

ssize_t my_getline(char **lineptr, size_t *n, FILE *stream);

/*
** NULL-terminated lines array
*/
char **read_lines_from_stream(FILE *stream, size_t *num_lines_ptr)
{
    char **lines = NULL;
    size_t num_lines = 0;

    char *lineptr = NULL;
    size_t lineptr_n;

    for (ssize_t num_read = 0;
         (num_read = my_getline(&lineptr, &lineptr_n, stream)) != -1;)
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

static char to_lower(char c);

int my_strcasecmp(const char *s1, const char *s2)
{
    // null str
    if (s1 == NULL)
    {
        return s2 == NULL ? 0 : to_lower(-*s2);
    }
    else if (s2 == NULL)
    {
        return to_lower(*s1);
    }
    // strcmp with to_lower calls
    int diff = 0;
    while ((diff = to_lower(*s1) - to_lower(*s2)) == 0 && *s1 != 0)
    {
        s1++;
        s2++;
    }
    return diff;
}

static char to_lower(char c)
{
    if ('A' <= c && c <= 'Z')
    {
        return c - 'A' + 'a';
    }
    return c;
}
