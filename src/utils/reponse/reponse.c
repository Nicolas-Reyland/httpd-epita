#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <utils/reponse/reponse.h>

static char *get_date_gmt(void);

static struct response *init_response(void);

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

void free_reponse(struct response *resp)
{
    if(!resp)
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
    time_t timestamp = time( NULL );
    struct tm * pTime = localtime( & timestamp );

    char *buffer = malloc(80);
    strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT\r\n", pTime );
    return buffer;
}

char *status_code(int *err)
{
    char *status_code = malloc(1);
    switch (*err)
    {
    case 400:
        status_code = realloc(status_code, strlen("HTTP/1.1 400 Bad Request\r\n")+1);
        strcpy(status_code, "HTTP/1.1 400 Bad Request\r\n");
        break;
    case 403:
        status_code = realloc(status_code, strlen("HTTP/1.1 403 Forbidden\r\n")+1);
        strcpy(status_code, "HTTP/1.1 403 Forbidden\r\n");
        break;
    case 404:
        status_code = realloc(status_code, strlen("HTTP/1.1 404 Not Found\r\n")+1);
        strcpy(status_code, "HTTP/1.1 404 Not Found\r\n");
        break;
    case 405:
        status_code = realloc(status_code, strlen("HTTP/1.1 405 Method Not Allowed\r\n")+1);
        strcpy(status_code, "HTTP/1.1 405 Method Not Allowed\r\n");
        break;
    case 505:
        status_code = realloc(status_code, strlen("HTTP/1.1 505 HTTP Version Not Supported\r\n")+1);
        strcpy(status_code, "HTTP/1.1 505 HTTP Version Not Supported\r\n");
        break;
    default:
        status_code = realloc(status_code, strlen("HTTP/1.1 200 OK\r\n")+1);
        strcpy(status_code, "HTTP/1.1 200 OK\r\n");
    }
    return status_code;
}

static void realloc_and_concat(struct response *resp, char *to_concat)
{
    if (resp->res_len == 0)
    {
        resp->res = to_concat;
        resp->res_len = strlen(to_concat);
        return;
    }
    resp->res = realloc(resp->res, strlen(to_concat)+resp->res_len+1);
    resp->res = strncat(resp->res,to_concat, strlen(to_concat)+1);
    resp->res_len += strlen(to_concat);
    free(to_concat);
}

char *create_response(int *err)
{    
    struct response *resp = init_response();
    if (!resp)
        return NULL;
    realloc_and_concat(resp,status_code(err));
    realloc_and_concat(resp,get_date_gmt());
    for(size_t i = 0; resp->res[i]!='\0'; i++)
    {
        unsigned char c = resp->res[i];
        if (c < ' ' || c == 0xf7)
            printf("<0x%x>", c);
        printf("%c", c);
    }
    free_reponse(resp);
    return NULL;
}

#if defined(CUSTOM_MAIN) && defined(MAIN2)
int main(void)
{
    int err = 404;
    char *res = create_response(&err);
    printf("%s",res);
    return 0;
}
#endif /* CUSTOM_MAIN */