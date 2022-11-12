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

static struct response *create_response(size_t *err, struct vhost *vhost,
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

struct response *create_response(size_t *err, struct vhost *vhost,
                                 struct request *req)
{
    struct response *resp = init_response();
    if (!resp)
        return NULL;
    set_status_code_header(err, resp); // set header
    set_date_gmt_header(resp); // set header date
    set_header_server_name(resp, vhost); // set header server_name
    char *path = get_path_ressource(req->target, vhost);
    if(!path)
    {
        *err = 404;
        return set_error_response(vhost, resp, err);
    }
    size_t size_ressource = 0;
    char *ressource = put_ressource_resp(path, &size_ressource, vhost, err);
    if (*err != 200) // in case of error, just send a response with the header
                     // and the date
    {
        free(path);
        free(ressource);
        return set_error_response(vhost, resp, err);
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

struct response *parsing_http(char *request_raw, size_t size,
                              struct vhost *vhost, ssize_t index)
{
    size_t err = 200;
    struct request *req = parser_request(request_raw, size, &err, index);
    log_request(vhost, req, &err, index);

    if (err != 200)
    {
        log_response(vhost, req, &err, index);
        struct response *resp = init_response();
        if (!resp)
            return NULL;
        free_request(req);
        return set_error_response(vhost, resp, &err);
    }
    struct response *resp = create_response(&err, vhost, req);
    log_response(vhost, req, &err, index);
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
