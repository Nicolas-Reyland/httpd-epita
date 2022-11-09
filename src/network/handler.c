#include "handler.h"

void register_connection(struct server_env *env)
{
    (void)env;
}

void process_data(struct server_env *env, int event_index, char *data,
                  size_t size)
{
    (void)env;
    (void)event_index;
    (void)data;
    (void)size;
}
