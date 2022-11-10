#include <stdbool.h>
#include <stdio.h>

#include "network/server.h"
#include "utils/hash_map/hash_map.h"
#include "utils/parsers/config/config_parser.h"

#ifndef CUSTOM_MAIN
int main(int argc, char **argv)
{
    if (argc < 2)
        return 4;

    struct server_config *config = parse_server_config(argv[1]);

    start_all(1, config);

    free_server_config(config, true);

    return 0;
}
#endif /* !CUSTOM_MAIN */
