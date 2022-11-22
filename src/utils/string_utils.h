#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdio.h>

char **read_lines_from_stream(FILE *stream, size_t *num_lines_ptr);

int is_empty_line(char *line);

void skip_all_classifier(char **content, int (*classifier)(int));

char *token_from_class(char **content, int (*classifier)(int),
                       size_t *token_len);

void skip_to_nonwhitespace(char **content);

int line_is_empty(char *line);

int replace_substring(char **str, char *str_start, char *substr,
                      char *replacement);

int my_strcasecmp(const char *s1, const char *s2);

#endif /* !STRING_UTILS_H */
