#ifndef HANDLER_H
#define HANDLER_H

#include "network/server_env.h"

void register_connection(struct server_env *env);

void process_data(struct server_env *env, int event_index, char *data,
                  size_t size);

#endif /* HANDLER_H */
