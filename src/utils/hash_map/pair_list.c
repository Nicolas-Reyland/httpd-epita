#include "pair_list.h"

#include <stddef.h>
#include <stdlib.h>

void free_pair_list(struct pair_list *list)
{
    if (list == NULL)
        return;
    free(list->key);
    free(list->value);
    free_pair_list(list->next);
    free(list);
}
