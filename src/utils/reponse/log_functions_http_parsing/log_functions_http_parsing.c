#include "utils/reponse/log_functions_http_parsing/log_functions_http_parsing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils/logging.h"
#include "utils/reponse/reponse.h"
#include "utils/state.h"

//--------------------------------------------------------------------------------
//------------------------------LOG
// functions-------------------------------------
//--------------------------------------------------------------------------------

static char *get_client_ip(struct vhost *vhost, ssize_t index);

void log_request(struct vhost *vhost, struct request *req, size_t *status_code,
                 ssize_t index)
{
    if (!g_state.logging)
        return;

    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char buffer[50];
    strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", pTime);
    char *client_ip = get_client_ip(vhost, index);

    char *serv_name = hash_map_get(vhost->map, "server_name");
    if (req && *status_code == 200)
    {
        log_message(LOG_STDOUT | LOG_EPITA,
                    "%s [%s] received %s on '%s' from %s\n", buffer, serv_name,
                    req->method, req->target, client_ip);
    }
    else
    {
        log_message(LOG_STDOUT | LOG_EPITA,
                    "%s [%s] received Bad Request from %s\n", buffer, serv_name,
                    client_ip);
    }
}

void log_response(struct vhost *vhost, struct request *req, size_t *status_code,
                  ssize_t index)
{
    if (!g_state.logging)
        return;

    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char buffer[50];
    strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", pTime);
    char *client_ip = get_client_ip(vhost, index);

    char *serv_name = hash_map_get(vhost->map, "server_name");
    if (req && *status_code != 400 && *status_code != 405)
    {
        log_message(LOG_STDOUT | LOG_EPITA,
                    "%s [%s] responding with %zu to %s for %s on '%s'\n",
                    buffer, serv_name, *status_code, client_ip, req->method,
                    req->target);
    }
    else if (*status_code == 400)
    {
        log_message(LOG_STDOUT | LOG_EPITA,
                    "%s [%s] responding with %zu to %s\n", buffer, serv_name,
                    *status_code, client_ip);
    }
    else if (*status_code == 405)
    {
        log_message(LOG_STDOUT | LOG_EPITA,
                    "%s [%s] responding with %zu to %s for UNKNOWN on '%s'\n",
                    buffer, serv_name, *status_code, client_ip, req->target);
    }
}

char *get_client_ip(struct vhost *vhost, ssize_t index)
{
    if (index == -1)
        return "???";

    return vhost->client_ips->data[index];
}

//------------------------------------------------------------------------------------
//------------------------------END LOG
// functions-------------------------------------
//------------------------------------------------------------------------------------
