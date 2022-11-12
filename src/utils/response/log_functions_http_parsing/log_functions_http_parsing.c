#include "utils/response/log_functions_http_parsing/log_functions_http_parsing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils/logging.h"
#include "utils/response/response.h"
#include "utils/state.h"

//--------------------------------------------------------------------------------
//------------------------------LOG
// functions-------------------------------------
//--------------------------------------------------------------------------------

static char *get_client_ip(struct client *client);

void log_request(struct client *client, struct request *req, int *status_code)
{
    if (!g_state.logging)
        return;

    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char buffer[50];
    strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", pTime);
    //char *client_ip = get_client_ip(vhost, index);
    char *client_ip = client->ip_addr;
    char *serv_name = hash_map_get(client->vhost->map, "server_name");
    if (req && *status_code == 200)
    {
        log_server("%s [%s] received %s on '%s' from %s\n", buffer, serv_name,
                   req->method, req->target, client_ip);
    }
    else
    {
        log_server("%s [%s] received Bad Request from %s\n", buffer, serv_name,
                   client_ip);
    }
}

void log_response(struct client *client, struct request *req, int *status_code)
{
    if (!g_state.logging)
        return;

    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char buffer[50];
    strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", pTime);
    char *client_ip = get_client_ip(client);

    char *serv_name = hash_map_get(client->vhost->map, "server_name");
    if (req && *status_code != 400 && *status_code != 405)
    {
        log_server("%s [%s] responding with %zu to %s for %s on '%s'\n", buffer,
                   serv_name, *status_code, client_ip, req->method,
                   req->target);
    }
    else if (*status_code == 400)
    {
        log_server("%s [%s] responding with %zu to %s\n", buffer, serv_name,
                   *status_code, client_ip);
    }
    else if (*status_code == 405)
    {
        log_server("%s [%s] responding with %zu to %s for UNKNOWN on '%s'\n",
                   buffer, serv_name, *status_code, client_ip, req->target);
    }
}

char *get_client_ip(struct client *client)
{
    if (!client || client->ip_addr == NULL)
        return "???";
    /*if (index_t >= vhost->client_ips->size)
        return "(unknown address)";*/

    return client->ip_addr;
}

//------------------------------------------------------------------------------------
//------------------------------END LOG
// functions-------------------------------------
//------------------------------------------------------------------------------------
