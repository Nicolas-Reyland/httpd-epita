#include "utils/reponse/reponse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "utils/logging.h"
#include "utils/state.h"

#define READ_BUFF_SIZE 4096

static void get_date_gmt(struct response *resp);

static struct response *init_response(void);

static void realloc_and_concat(struct response *resp, char *to_concat,
                               size_t to_concat_len, bool free_obj);

static void status_code(size_t *err, struct response *resp);

static char *put_ressource_resp(char *path, size_t *err, size_t *size);

/*
 *   void paramater
 *   Function: Init the struct response
 */
struct response *init_response(void)
{
    struct response *response = malloc(sizeof(struct response));
    if (!response)
        return NULL;
    response->err = 0;
    response->res = NULL;
    response->res_len = 0;
    return response;
}

void free_response(struct response *resp)
{
    if (!resp)
        return;
    free(resp->res);
    free(resp);
}

/*
 *   void paramater
 *   Function: return a string in format
 *            "Mon, 19 Sep 2022 15:52:45 GMT"
 */
void get_date_gmt(struct response *resp)
{
    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char *buffer = malloc(50);
    strftime(buffer, 50, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", pTime);
    size_t size = strlen(buffer);
    realloc_and_concat(resp, buffer, size, true);
}

/*
 *   err = code error set by the function request
 *   Function: return a string in format follow the format
 *              " HTTP-Version SP Status-Code SP Reason-Phrase CRLF"
 */
void status_code(size_t *err, struct response *resp)
{
    size_t size = 0;
    char *status_code = NULL;
    switch (*err)
    {
    case 400:
        status_code = "HTTP/1.1 400 Bad Request\r\n";
        break;
    case 403:
        status_code = "HTTP/1.1 403 Forbidden\r\n";
        break;
    case 404:
        status_code = "HTTP/1.1 404 Not Found\r\n";
        break;
    case 405:
        status_code = "HTTP/1.1 405 Method Not Allowed\r\n";
        break;
    case 505:
        status_code = "HTTP/1.1 505 HTTP Version Not Supported\r\n";
        break;
    default:
        status_code = "HTTP/1.1 200 OK\r\n";
    }
    size = strlen(status_code);
    realloc_and_concat(resp, status_code, size, false);
}

/*
 *   target = ressource that the client want to get
 *   vhost = vhost that the server gave to extract the root dir
 *   Function: return the full path of the ressource concat with
 *             the root dir from vhost
 */
static char *get_path_ressource(char *target, struct vhost *vhost)
{
    char *root_dir = hash_map_get(vhost->map, "root_dir");
    char *path = malloc(strlen(root_dir) + 1);
    path = strcpy(path, root_dir);
    path = realloc(path, strlen(path) + strlen(target) + 1);
    path = strcat(path, target);
    return path;
}

/*
 *   path = the full path of the ressource
 *   vhost = the structure response w/ headers
 *   Function: concatain the response with the contain of the file given in
 * parameter and return the response
 */
char *put_ressource_resp(char *path, size_t *err, size_t *size)
{
    // if we can open the file:
    FILE *file = fopen(path, "r");
    if (access(path, R_OK) == -1)
    {
        if (access(path, F_OK) == -1)
        {
            *err = 404;
            return NULL;
        }
        else
        {
            *err = 403;
            return NULL;
        }
    }
    char buff[READ_BUFF_SIZE];
    //--------puts /r/n one more time between headers and ressources
    // realloc_and_concat(resp, "\r\n", false);
    //--------
    char *res = NULL;
    ssize_t nb_read = fread(buff, 1, READ_BUFF_SIZE, file);
    while (nb_read > 0)
    {
        res = realloc(res, *size + nb_read);
        res = memcpy(res + *size, buff, nb_read);
        *size += nb_read;
        nb_read = fread(buff, 1, READ_BUFF_SIZE, file);
    }
    fclose(file);
    return res;
}

/*
 *   resp = structure response
 *   to_concat = string that we want to concatain with the string response
 *   Function: concatain string resp->res w/ the string to concat
 *              then free the to_concat string
 */
void realloc_and_concat(struct response *resp, char *to_concat,
                        size_t to_concat_len, bool free_obj)
{
    if (resp->res_len == 0)
    {
        resp->res = memcpy(malloc(to_concat_len), to_concat, to_concat_len);
        resp->res_len = to_concat_len;
        return;
    }
    resp->res = realloc(resp->res, to_concat_len + resp->res_len);
    memcpy(resp->res + resp->res_len, to_concat, to_concat_len);
    resp->res_len += to_concat_len;
    if (free_obj)
        free(to_concat);
}

static void set_header_content_length(size_t content_len, struct response *resp)
{
    char *res = malloc(50);
    sprintf(res, "Content-Length: %zu\r\n", content_len);
    size_t size = strlen(res);
    realloc_and_concat(resp, res, size, true);
}

static void connexion_close_header(struct response *resp)
{
    char *close = "Connection: close\r\n";
    size_t size = strlen(close);
    realloc_and_concat(resp, close, size, false);
}

static struct response *set_error_response(char *path, char *ressource,
                                           struct response *resp, size_t *err)
{
    free(path);
    free(ressource);
    free(resp->res);
    resp->res_len = 0;
    resp->res = NULL;
    status_code(err, resp); // set header
    get_date_gmt(resp); // set header date
    connexion_close_header(resp); // set header connexion close
    set_header_content_length(0, resp); // set content len header 0
    realloc_and_concat(resp, "\r\n", 2, false);
    return resp;
}

struct response *create_response(size_t *err, struct vhost *vhost,
                                 struct request *req)
{
    struct response *resp = init_response();
    if (!resp)
        return NULL;
    status_code(err, resp); // set header
    get_date_gmt(resp); // set header date
    char *path = get_path_ressource(req->target, vhost);
    size_t size_ressource = 0;
    char *ressource = put_ressource_resp(path, err, &size_ressource);
    if (*err != 200) // in case of error, just send a response with the header
                     // and the date
    {
        return set_error_response(path, ressource, resp, err);
    }
    set_header_content_length(size_ressource, resp); // set header content len
    realloc_and_concat(resp, "\r\n", 2, false);
    if (strcmp(req->method, "GET") == 0)
    {
        realloc_and_concat(resp, ressource, size_ressource,
                           true); // put ressource into response
        free(path);
        return resp;
    }
    free(ressource);
    free(path);
    return resp;
}

static void log_request(struct vhost *vhost, struct request *req, size_t *status_code)
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

static void log_response(struct vhost *vhost, struct request *req, size_t *status_code)
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

struct response *parsing_http(char *request_raw, size_t size,
                              struct vhost *vhost)
{
    size_t err = 200;
    struct request *req = parser_request(request_raw, size, &err);
    log_request(vhost,req, &err);

    if (err != 200)
    {
        log_response(vhost, req, &err);
        struct response *resp = init_response();
        if (!resp)
            return NULL;
        free_request(req);
        return set_error_response(NULL, NULL, resp, &err);
    }
    struct response *resp = create_response(&err, vhost, req);
    log_response(vhost, req, &err);
    free_request(req);
    return resp;
}

#if defined(CUSTOM_MAIN) && defined(MAIN2)
int main(void)
{
    size_t err = 200;
    char req_string[] = "GET test.txt "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host: 123:456\0\r\n"
                        "\r\n"
                        "this \0is t\0\0he body";
    size_t size = sizeof(req_string) - 1;
    struct request *req = parser_request(req_string, size, &err);
    struct response *res = create_response(&err, "./", req);
    free_request(req);
    free_response(res);
    return 0;
}
#endif /* CUSTOM_MAIN */