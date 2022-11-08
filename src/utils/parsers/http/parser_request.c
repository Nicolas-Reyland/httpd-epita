#include "parser_request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t end_token(char *request, size_t index);

static size_t next_token(char *request, size_t index);

static char *my_strcpy(char *request, size_t begin, size_t end);

static void free_request(struct request *req);

static void fill_struct(struct request *req, char *token, size_t count);

static struct request *init_request(void);

/*
 *   void paramater
 *   Function: Init the struct request
 */
struct request *init_request(void)
{
    struct request *req = malloc(sizeof(struct request));
    if (!req)
        return NULL;
    req->method = NULL;
    req->target = NULL;
    req->version = NULL;
    return req;
}

/*
 *   request = string of the http request
 *   index = index where we ended the parsing
 *   Function: Search the index of the end of the token
 */
size_t end_token(char *request, size_t index)
{
    size_t i = index;
    while (request[i] != '\0' && request[i] != ' ')
        i++;
    return i;
}

/*
 *   request = string of the http request
 *   index = index where we begin to parse
 *   Function: Search the index of the next token
 */
size_t next_token(char *request, size_t index)
{
    size_t i = index;
    while (request[i] == ' ')
        i++;
    return i;
}

/*
 *   request = string of the http request
 *   begin = index where the token begin
 *   Function: Copy the caracters between begin and end
 */
char *my_strcpy(char *request, size_t begin, size_t end)
{
    char *dest = malloc(sizeof(char) * (end - begin + 1));
    size_t j = 0;
    for (size_t i = begin; i < end; i++)
    {
        dest[j] = request[i];
        j++;
    }
    dest[j] = '\0';
    return dest;
}

/*
 *   request = curr struct request
 *   Function: free the struct request
 */
void free_request(struct request *req)
{
    if (req->method)
        free(req->method);
    if (req->target)
        free(req->target);
    if (req->version)
        free(req->version);
    free(req);
}

/*
 *  request = curr struct request
 *   token = string that contain the Method||target||version
 *   count = It is the buffer that say where we are in the request and where
 *           we must place the token in our structure request
 *   Function: fill the structure request with the token
 *             It fills in relation to count
 */
void fill_struct(struct request *req, char *token, size_t count)
{
    if (count == 0)
        req->method = token;
    else if (count == 1)
        req->target = token;
    else
    {
        req->version = token;
    }
}

/*
 *   request = curr struct request
 *   Function: Fill and return a struct request with the string
 *             given in parameter. In case of error, return NULL
 */
struct request *parse_request(char *request)
{
    struct request *req = init_request();
    if (!req)
        return NULL;
    size_t count = 0;
    for (size_t i = 0; request[i] != '\0';)
    {
        i = next_token(request, i);
        if (request[i] == '\0')
            break;
        size_t end = end_token(request, i);
        char *token = my_strcpy(request, i, end);
        if (!token)
        {
            free_request(req);
            return NULL;
        }
        fill_struct(req, token, count);
        count++;
        i = end;
    }
    if (count != 3)
    {
        free_request(req);
        return NULL;
    }
    return req;
}
