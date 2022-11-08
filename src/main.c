#include <stdbool.h>
#include <stdio.h>

#include "utils/hash_map/hash_map.h"
#include "utils/parsers/config/config_parser.h"

#ifndef CUSTOM_MAIN
int main(int argc, char **argv)
{
    if (argc < 2)
        return 4;

    struct server_config *config = parse_server_config(argv[1]);
    if (config == NULL)
    {
        puts("(null)");
        return 0;
    }

    puts("global :");
    hash_map_dump(config->global, " - ");
    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        printf("vhosts %zu :\n", i);
        hash_map_dump(config->vhosts[i], " - ");
    }

    free_server_config(config, true);

    return 0;
}
#endif /* !CUSTOM_MAIN */
