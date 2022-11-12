#include "utils/response/response.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "utils/logging.h"
#include "utils/response/log_functions_http_parsing/log_functions_http_parsing.h"
#include "utils/response/set_header_response/set_header_response.h"
#include "utils/response/tools_response/tools_response.h"
#include "utils/state.h"

static struct response *init_response(void);

static struct response *create_response(int *err, struct client *client,
                                        struct request *req);

/*
 *   void paramater
 *   Function: Init the struct response
 */
struct response *init_response(void)
{
    struct response *response = malloc(sizeof(struct response));
    if (!response)
        return NULL;
    response->err = 200;
    response->res = NULL;
    response->res_len = 0;
    return response;
}

void free_response(struct response *resp)
{
    if (!resp)
        return;
    if (resp->fd != -1)
        close(resp->fd);
    free(resp->res);
    free(resp);
}

struct response *create_response(int *err, struct client *client,
                                 struct request *req)
{
    struct response *resp = init_response();
    if (!resp)
        return NULL;
    set_status_code_header(err, resp); // set header
    set_date_gmt_header(resp); // set header date
    set_header_server_name(resp, client->vhost); // set header server_name
    char *path = get_path_ressource(req->target, client->vhost);
    if (!path)
    {
        *err = 404;
        return set_error_response(client->vhost, resp, err);
    }
    if (*err != 200) // in case of error, just send a response with the header
                     // and the date
    {
        free(path);
        return set_error_response(client->vhost, resp, err);
    }

    // Access ressource (file)
    int open_ressource_result = open_ressource(path, resp, client->vhost,
                                               strcmp(req->method, "GET") == 0);
    *err = resp->err free(path);
    if (open_ressource_result == -1)
        return set_error_response(client->vhost, resp, &resp->err);

    // set header content len
    set_header_content_length(resp->file_len, resp);
    realloc_and_concat(resp, "\r\n", 2, false);

    return resp;
}

struct response *parsing_http(char *request_raw, size_t size,
                              struct client *client)
{
    int err = 200;
    struct request *req = parser_request(request_raw, size, &err, vhost);
    log_request(client, req, &err);

    if (err != 200)
    {
        log_response(client, req, &err);
        struct response *resp = init_response();
        if (!resp)
            return NULL;
        free_request(req);
        return set_error_response(client->vhost, resp, &err);
    }
    struct response *resp = create_response(&err, client, req);
    log_response(client, req, &err);
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
