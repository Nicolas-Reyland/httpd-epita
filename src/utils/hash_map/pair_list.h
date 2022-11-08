#ifndef PAIR_LIST_H
#define PAIR_LIST_H

struct pair_list
{
    char *key;
    char *value;
    struct pair_list *next;
};

void free_pair_list(struct pair_list *list);

#endif /* !PAIR_LIST_H */
