#include "utils/response/set_header_response/set_header_response.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils/logging.h"
#include "utils/response/response.h"

//--------------------------------------------------------------------------------
//------------------------------Set header
// functions------------------------------
//--------------------------------------------------------------------------------
/*
 *   err = code error set by the function request
 *   Function: return a string in format follow the format
 *              " HTTP-Version SP Status-Code SP Reason-Phrase CRLF"
 */
void set_status_code_header(int(*err), struct response *resp)
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

void set_header_content_length(size_t content_len, struct response *resp)
{
    char *res = alloca(50);
    sprintf(res, "Content-Length: %zu\r\n", content_len);
    size_t size = strlen(res);
    log_debug("Writing \"%s\" to resp\n", res);
    realloc_and_concat(resp, res, size, false);
}

void connexion_close_header(struct response *resp)
{
    char *close = "Connection: close\r\n";
    size_t size = strlen(close);
    realloc_and_concat(resp, close, size, false);
}

void set_header_server_name(struct response *resp, struct vhost *vhost)
{
    char *serv_name = hash_map_get(vhost->map, "server_name");
    if (serv_name)
    {
        char *res = alloca(50);
        sprintf(res, "Server: %s\r\n", serv_name);
        size_t size = strlen(res);
        realloc_and_concat(resp, res, size, false);
    }
}

struct response *set_error_response(struct vhost *vhost, struct response *resp,
                                    int(*err))
{
    free(resp->res);
    resp->res_len = 0;
    resp->res = NULL;
    resp->close_connection = 1;
    set_status_code_header(err, resp); // set header
    set_date_gmt_header(resp); // set header date
    set_header_server_name(resp, vhost); // set header server name
    connexion_close_header(resp); // set header connexion close
    set_header_content_length(0, resp); // set content len header 0
    realloc_and_concat(resp, "\r\n", 2, false);
    return resp;
}

/*
 *   void paramater
 *   Function: return a string in format
 *            "Mon, 19 Sep 2022 15:52:45 GMT"
 */
void set_date_gmt_header(struct response *resp)
{
    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);

    char *buffer = alloca(50);
    strftime(buffer, 50, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", pTime);
    size_t size = strlen(buffer);
    realloc_and_concat(resp, buffer, size, false);
}

//--------------------------------------------------------------------------------
//------------------------------END set header
// functions--------------------------
//--------------------------------------------------------------------------------
