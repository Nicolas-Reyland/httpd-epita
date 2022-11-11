#include "utils/reponse/reponse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/logging.h"
#include "utils/state.h"
#include "utils/reponse/log_functions_http_parsing/log_functions_http_parsing.h"

//--------------------------------------------------------------------------------
//------------------------------LOG functions-------------------------------------
//--------------------------------------------------------------------------------
void log_request(struct vhost *vhost, struct request *req, size_t *status_code)
{
    char *log_value = hash_map_get(g_state.env->config->global,"log");
    int logging = log_value != NULL && strcmp(log_value, "true") == 0;
    if (!logging)
    {
        return;
    }
    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char *buffer = malloc(50);
    strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", pTime);
    buffer = realloc(buffer,strlen(buffer)+1);
    
    char *serv_name = hash_map_get(vhost->map,"server_name");
    if (req && *status_code == 200)
    {
        log_message(LOG_STDOUT | LOG_EPITA, "%s [%s] received %s on '%s' from 0.0.0.0\n", buffer,serv_name,req->method, req->target);
    }
    else
    {
        log_message(LOG_STDOUT | LOG_EPITA, "%s [%s] received Bad Request from 0.0.0.0\n", buffer,serv_name);
    }
    free(buffer);
}

void log_response(struct vhost *vhost, struct request *req, size_t *status_code)
{
    char *log_value = hash_map_get(g_state.env->config->global,"log");
    int logging = log_value != NULL && strcmp(log_value, "true") == 0;
    if (!logging)
    {
        return;
    }
    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char *buffer = malloc(50);
    strftime(buffer, 50, "%a, %d %b %Y %H:%M:%S GMT", pTime);
    buffer = realloc(buffer,strlen(buffer)+1);
    
    char *serv_name = hash_map_get(vhost->map,"server_name");
    if (req && *status_code != 400 && *status_code != 405)
    {
        log_message(LOG_STDOUT | LOG_EPITA, "%s [%s] responding with %zu to 0.0.0.0 for %s on '%s'\n", buffer,serv_name,*status_code, req->method, req->target);
    }
    else if(*status_code == 400)
    {
        log_message(LOG_STDOUT | LOG_EPITA, "%s [%s] responding with %zu to 0.0.0.0\n", buffer,serv_name,*status_code);
    }
    else if(*status_code == 405)
    {
        log_message(LOG_STDOUT | LOG_EPITA, "%s [%s] responding with %zu to 0.0.0.0 for UNKNOWN on '%s'\n", buffer,serv_name,*status_code, req->target);
    }
    free(buffer);
}
//------------------------------------------------------------------------------------
//------------------------------END LOG functions-------------------------------------
//------------------------------------------------------------------------------------