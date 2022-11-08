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

int replace_substring(char **str, char *str_start, char *substr,
                      char *replacement);

#endif /* !STRING_UTILS_H */
