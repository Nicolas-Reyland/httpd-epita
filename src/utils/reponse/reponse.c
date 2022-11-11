#include "utils/reponse/reponse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define READ_BUFF_SIZE 4096

static char *get_date_gmt(void);

static struct response *init_response(void);

static void realloc_and_concat(struct response *resp, char *to_concat,
                               bool free_obj);

static char *status_code(size_t *err);

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
char *get_date_gmt(void)
{
    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char *buffer = malloc(80);
    strftime(buffer, 80, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", pTime);
    return buffer;
}

/*
 *   err = code error set by the function request
 *   Function: return a string in format follow the format
 *              " HTTP-Version SP Status-Code SP Reason-Phrase CRLF"
 */
char *status_code(size_t *err)
{
    char *status_code = malloc(1);
    switch (*err)
    {
    case 400:
        status_code =
            realloc(status_code, strlen("HTTP/1.1 400 Bad Request\r\n") + 1);
        strcpy(status_code, "HTTP/1.1 400 Bad Request\r\n");
        break;
    case 403:
        status_code =
            realloc(status_code, strlen("HTTP/1.1 403 Forbidden\r\n") + 1);
        strcpy(status_code, "HTTP/1.1 403 Forbidden\r\n");
        break;
    case 404:
        status_code =
            realloc(status_code, strlen("HTTP/1.1 404 Not Found\r\n") + 1);
        strcpy(status_code, "HTTP/1.1 404 Not Found\r\n");
        break;
    case 405:
        status_code = realloc(
            status_code, strlen("HTTP/1.1 405 Method Not Allowed\r\n") + 1);
        strcpy(status_code, "HTTP/1.1 405 Method Not Allowed\r\n");
        break;
    case 505:
        status_code =
            realloc(status_code,
                    strlen("HTTP/1.1 505 HTTP Version Not Supported\r\n") + 1);
        strcpy(status_code, "HTTP/1.1 505 HTTP Version Not Supported\r\n");
        break;
    default:
        status_code = realloc(status_code, strlen("HTTP/1.1 200 OK\r\n") + 1);
        strcpy(status_code, "HTTP/1.1 200 OK\r\n");
    }
    return status_code;
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
    char *res = malloc(1);
    res[0] = '\0';
    ssize_t nb_read = fread(buff, 1, READ_BUFF_SIZE, file);
    while (nb_read > 0)
    {
        *size += nb_read;
        res = realloc(res, *size + 2);
        res = strncat(res, buff, nb_read + 1);
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
void realloc_and_concat(struct response *resp, char *to_concat, bool free_obj)
{
    if (resp->res_len == 0)
    {
        resp->res = to_concat;
        resp->res_len = strlen(to_concat);
        return;
    }
    resp->res = realloc(resp->res, strlen(to_concat) + resp->res_len + 1);
    resp->res = strncat(resp->res, to_concat, strlen(to_concat) + 1);
    resp->res_len += strlen(to_concat);
    if (free_obj)
        free(to_concat);
}

static char *get_header_content_length(size_t content_len)
{
    char *res = malloc(39);
    sprintf(res, "Content-Length: %zu\r\n", content_len);
    res = realloc(res, strlen(res) + 1);
    return res;
}

static struct response *set_error_response(char *path, char *ressource,
                                           struct response *resp, size_t *err)
{
    free(path);
    free(ressource);
    free(resp->res);
    resp->res_len = 0;
    resp->res = NULL;
    realloc_and_concat(resp, status_code(err), true);
    realloc_and_concat(resp, get_date_gmt(), true);
    realloc_and_concat(resp, "Connection: close\r\n", false);
    char *content_len = get_header_content_length(0);
    realloc_and_concat(resp, content_len, true);
    realloc_and_concat(resp, "\r\n", false);
    return resp;
}

struct response *create_response(size_t *err, struct vhost *vhost,
                                 struct request *req)
{
    struct response *resp = init_response();
    if (!resp)
        return NULL;
    realloc_and_concat(resp, status_code(err), true); // set header
    realloc_and_concat(resp, get_date_gmt(), true); // set header date
    char *path = get_path_ressource(req->target, vhost);
    size_t size_ressource = 0;
    char *ressource = put_ressource_resp(path, err, &size_ressource);
    if (*err != 200) // in case of error, just send a response with the header
                     // and the date
        return set_error_response(path, ressource, resp, err);
    char *content_len =
        get_header_content_length(size_ressource); // take ressource size
    realloc_and_concat(resp, content_len,
                       true); // create content-len: len header
    if (strcmp(req->method, "GET") == 0)
    {
        realloc_and_concat(resp, ressource,
                           true); // put ressource into response
        free(path);
        return resp;
    }
    free(ressource);
    free(path);
    return resp;
}

struct response *parsing_http(char *request_raw, size_t size,
                              struct vhost *vhost)
{
    size_t err = 200;
    struct request *req = parser_request(request_raw, size, &err);
    if (err != 200)
    {
        free_request(req);
        struct response *resp = init_response();
        if (!resp)
            return NULL;
        realloc_and_concat(resp, status_code(&err), true);
        realloc_and_concat(resp, get_date_gmt(), true);
        return resp;
    }
    struct response *resp = create_response(&err, vhost, req);
    free_request(req);
    return resp;
}

#if defined(CUSTOM_MAIN) && defined(MAIN2)
int main(void)
{
    int err = 200;
    struct response *res = create_response(&err, "./", "test.txt");
    free_response(res);
    return 0;
}
#endif /* CUSTOM_MAIN */