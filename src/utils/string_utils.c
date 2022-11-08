#include "utils/string_utils.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
** NULL-terminated lines array
*/
char **read_lines_from_stream(FILE *stream, size_t *num_lines_ptr)
{
    char **lines = NULL;
    size_t num_lines = 0;

    char *lineptr = NULL;
    size_t lineptr_n = 0;

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
        lineptr_n = 0;
    }
    free(lineptr);
    if (!feof(stream) || ferror(stream))
    {
        // an error occured while reading from the stream
        for (size_t i = 0; i < num_lines; ++i)
            free(lines[i]);
        free(lines);
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

char *get_next_word(char **content, const char *delim)
{
    skip_to_nonwhitespace(content);

    // set start of token
    char *start = *content;
    // increment content, until a delimiter from 'delim'
    // or a whitespace is met (see isspace(3))
    for (; **content != 0; ++(*content))
    {
        // loop over the delimiters, breaking if one matches teh current char
        const char *d = delim;
        for (; *d != 0; d++)
            if (**content == *d)
                break;
        // checking if a delimiter or a whitespace has been met
        if (*d != 0 || isspace(**content))
            break;
    }

    if (start == *content)
        return NULL;

    // copy content into token
    size_t token_size = *content - start;
    char *token = malloc(token_size + 1);
    memcpy(token, start, token_size);
    token[token_size] = 0;

    return token;
}

void skip_to_nonwhitespace(char **content)
{
    skip_all_classifier(content, isspace);
}

int is_empty_line(char *line)
{
    skip_to_nonwhitespace(&line);
    return *line == 0 || *line == '#';
}

void skip_all_classifier(char **content, int (*classifier)(int))
{
    for (; **content != 0 && (*classifier)(**content); ++(*content))
        continue;
}

char *token_from_class(char **content, int (*classifier)(int),
                       size_t *token_len)
{
    char *content_start = *content;
    for (; **content != 0 && (*classifier)(**content); ++(*content))
        continue;
    size_t num_chars = *content - content_start;
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

int is_space_char(int c)
{
    return c == ' ';
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

char *get_location_after_char(char *str, char c)
{
    for (char *x = str; *x != 0; ++x)
        if (*x == c)
            return x + 1;

    return NULL;
}
