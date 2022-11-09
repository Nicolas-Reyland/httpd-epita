#include "parser_request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/mem.h"
#include "utils/string_utils.h"

static size_t end_token(char *request, size_t index);

static size_t next_token(char *request, size_t index);

static char *my_strcpy(char *request, size_t begin, size_t end);

static void free_request(struct request *req);

static void fill_struct(struct request *req, char *token, size_t count);

static struct request *init_request(void);

static struct request *parse_request_header(char *request);

static void tokenise_option(char *token, struct request *request);

static int is_not_cr(int c);

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
    req->body_size = 0;
    req->body = NULL;
    req->hash_map = hash_map_init(1);
    if (!req->hash_map)
    {
        free(req);
        return NULL;
    }
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
    if (req == NULL)
        return;

    if (req->method)
        free(req->method);
    if (req->target)
        free(req->target);
    if (req->version)
        free(req->version);
    if (req->body)
        free(req->body);
    if (req->hash_map)
        free_hash_map(req->hash_map,1);
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
 *   Function: Create and fill a struct request with the string
 *             header line given in parameter. Return NULL in
 *             case of error
 */
struct request *parse_request_header(char *request)
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

/*
 *   token = string containing "key: value"
 *   request = The request we are parsing
 *   Function: Tokenise a string containing "key: value" and puts
 *              it in hashmap of request
 */
void tokenise_option(char *token, struct request *request)
{
    if (!token)
        return;
    size_t i = 0;
    size_t end = 0;
    i = next_token(token, i);
    end = end_token(token, i);
    if (token[i] == '\0')
        return;
    char *key = my_strcpy(token, i, end - 1);
    i = end;
    i = next_token(token, i);
    end = end_token(token, i);
    if (token[i] == '\0')
        return;
    char *value = my_strcpy(token, i, end);

    hash_map_insert(request->hash_map, key, value, NULL);
}

/*
 *   c = character to test
 *   Function: Test if c is a carriage return
 */
int is_not_cr(int c)
{
    return c != '\r';
}

/*
 *   request = request string to parse
 *   token = current token
 *   req = struct request of the string request   
 *   Function: free different elements
 */
void free_elements(char *request, char *token, struct request *req)
{
    free(token);
    free(request);
    free_request(req);
}

static char *sanitize_request(char *raw_request, size_t *size);

/*
 *   request = request string to parse
 *   Function: parse a request string and
 *             return a struct request fullfilled
 */
struct request *parser_request(char *raw_request, size_t size)
{
    (void)size;
    char *request = sanitize_request(raw_request, &size);
    char *initial_ptr = request;

    char *token = token_from_class(&request, is_not_cr, NULL);
    if (!token)
    {
        free_elements(initial_ptr, NULL, NULL);
        return NULL;
    }
    struct request *req = parse_request_header(token);
    if (!req)
    {
        free_elements(initial_ptr, token, req);
        return NULL;
    }

    request += 2;
    while (token != NULL && request[0] != '\r')
    {
        FREE_SET_NULL(token);
        token = token_from_class(&request, is_not_cr, NULL);
        tokenise_option(token, req);
        request += 2;
    }
    request += 2;

    size_t off_to_data = request - initial_ptr;
    size_t size_of_data = size - off_to_data;
    req->body_size = size_of_data;
    memmove(initial_ptr, request, size_of_data);
    initial_ptr = realloc(initial_ptr, size_of_data);
    req->body = initial_ptr;

    free_elements(token, NULL, NULL);
    return req;
}

char *sanitize_request(char *raw_request, size_t *size)
{
    char *clean_req = malloc(*size);
    char stop_at[4] = "\r\n\r\n";
    size_t clean_i = 0;
    for (size_t i = 0; i < *size - 4; ++i)
    {
        if (memcmp(raw_request + i, stop_at, 4) == 0)
        {
            memcpy(clean_req + clean_i, raw_request + i, *size - i);
            *size -= i - clean_i;
            return clean_req;
        }
        if (raw_request[i] != '\0')
            clean_req[clean_i++] = raw_request[i];
    }

    // No CRLF CRLF found
    return NULL;
}

#if defined(CUSTOM_MAIN) && defined(MAIN1)
int main(void)
{
    char req_string[] = "GET /path/script.cgi?field1=value1&field2=value2 "
                       "H\0\0\0TTP/1.1\r\n"
                       "con\0nexion: close\r\n"
                       "insh: c\0amarche\r\n"
                       "key     :\0v\0\r\n"
                       "\r\n"
                       "this \0is t\0\0he body";
    size_t size = sizeof(req_string) - 1;
    struct request *req =
        parser_request(req_string, size);
    if (req)
    {
        printf("%s\n", req->method);
        printf("%s\n", req->target);
        printf("%s\n", req->version);
        for (size_t i = 0; i < req->body_size; ++i) {
            unsigned char c = req->body[i];
            if (c < ' ' || c == 0xf7)
                printf("<0x%x>", c);
            printf("%c", c);
        }
        printf("\n");
        printf("%zu\n", req->hash_map->num_keys);
        hash_map_dump(req->hash_map, " - ");
        free_request(req);
    }
    return 0;
}
#endif /* CUSTOM_MAIN */
