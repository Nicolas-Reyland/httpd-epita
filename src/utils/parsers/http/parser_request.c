#include "parser_request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/mem.h"
#include "utils/parsers/error_parsing/error_parsing.h"
#include "utils/string_utils.h"

static size_t end_token(char *request, size_t index, char *delim);

static size_t next_token(char *request, size_t index, char *delim);

static char *my_strcpy(char *request, size_t begin, size_t end);

static struct request *init_request(void);

static struct request *parse_request_header(char *request);

static void tokenise_option(char *token, struct request *request, int *err);

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
size_t end_token(char *request, size_t index, char *delim)
{
    size_t i = index;
    if (!delim)
    {
        while (request[i] != '\0' && request[i] != ' ')
            i++;
        return i;
    }
    while (request[i] != '\0' && request[i] != ' ' && request[i] != *delim)
        i++;
    return i;
}

/*
 *   request = string of the http request
 *   index = index where we begin to parse
 *   delim = an aditional delim that you can give or not
 *   Function: Search the index of the next token
 */
size_t next_token(char *request, size_t index, char *delim)
{
    size_t i = index;
    if (delim)
    {
        while (request[i] == ' ' || request[i] == *delim)
            i++;
        return i;
    }
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
        free_hash_map(req->hash_map, 1);
    free(req);
}

/*
 *   request = The string of the headers to parse
 *   begin   = index where we are in the given string request (begin == end)
 *   end   = index where we are in the given string request (begin == end)
 *   Function: parse the given string of the headers in parameter
 */
static char *parse_header(char *request, size_t *begin, size_t *end)
{
    *begin = next_token(request, *begin, NULL);
    if (request[*begin] == '\0')
        return NULL;
    *end = end_token(request, *begin, NULL);
    char *header = my_strcpy(request, *begin, *end);
    *begin = *end;
    return header;
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
    size_t begin = 0;
    size_t end = 0;
    req->method = parse_header(request, &begin, &end);
    req->target = parse_header(request, &begin, &end);
    req->version = parse_header(request, &begin, &end);

    if (!req->method || !req->target || !req->version)
    {
        free_request(req);
        return NULL;
    }
    return req;
}

/*
 *   token = curr token that we are parsing
 *   i = where we are in the string token, at the begining i == end
 *   end = where we are in the string token, at the begining i == end
 *   Function: tokenise the key of a given string key: value
 */
static char *tokenise_key(char *token, size_t *i, size_t *end)
{
    if (!token)
        return NULL;
    *i = next_token(token, *i, NULL);
    char c = ':';
    *end = end_token(token, *i, &c);
    if (token[*i] == '\0')
        return NULL;
    char *key = NULL;
    if (token[(*end)] == ':')
    {
        key = my_strcpy(token, *i, *end);
    }
    *i = *end + 1;
    return key;
}

/*
 *   token = curr token that we are parsing
 *   i = where we are in the string token, at the begining i == end
 *   end = where we are in the string token, at the begining i == end
 *   Function: tokenise the value of a given string key: value
 */
static char *tokenise_value(char *token, size_t *i, size_t *end)
{
    if (!token)
        return NULL;
    *i = next_token(token, *i, NULL);
    *end = end_token(token, *i, NULL);
    if (token[*i] == '\0')
        return NULL;
    char *key = my_strcpy(token, *i, *end);
    *i = *end;
    return key;
}

/*
 *   token = string containing "key: value"
 *   request = The request we are parsing
 *   Function: Tokenise a string containing "key: value" and puts
 *              it in hashmap of request
 */
void tokenise_option(char *token, struct request *request, int *err)
{
    if (!token)
        return;
    size_t i = 0;
    size_t end = 0;
    char *key = tokenise_key(token, &i, &end);
    if (!key)
    {
        *err = 1;
        return;
    }
    char *value = tokenise_value(token, &i, &end);
    if (!value)
    {
        *err = 1;
        return;
    }
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

static char *sanitize_request(char *raw_request, size_t *size)
{
    if (*size < 3 || raw_request == NULL)
        return NULL;

    char *clean_req = malloc(*size);
    char stop_at[4] = "\r\n\r\n";
    size_t clean_i = 0;

    for (size_t i = 0; i < *size - 3; ++i)
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
    free(clean_req);
    return NULL;
}

/*
 *   request = request string to parse
 *   size = length of the request
 *   Function: parse a request string and
 *             return a struct request fullfilled
 */
struct request *sub_parser_request(char *raw_request, size_t size)
{
    char *request = sanitize_request(raw_request, &size);
    if (!request)
        return NULL;
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
    int err = 0;
    while (token != NULL && request[0] != '\r')
    {
        FREE_SET_NULL(token);
        token = token_from_class(&request, is_not_cr, NULL);
        tokenise_option(token, req, &err);
        if (err == 1)
        {
            free_elements(token, initial_ptr, req);
            return NULL;
        }
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

/*
 *   options = options of the structure request
 *   Function: verify if there is the options host in req->options
 *              in case of error, set the *err and return the error number
 */
static int not_contain_host(struct pair_list *options, int *err)
{
    while (options)
    {
        if (strcmp(options->key, "Host") == 0)
        {
            return 0;
        }
        options = options->next;
    }
    *err = HOST_ERR;
    return HOST_ERR;
}

/*
 *   version = version of the structure request
 *   Function: verify if the protocol is valid in req->version
 *              in case of error, set the *err and return the error number
 */
static int is_not_protocol_valid(char *version, int *err)
{
    if (strcmp(version, "HTTP/1.1") == 0)
    {
        return 0;
    }
    *err = VERSION_ERR;
    return VERSION_ERR;
}

/*
 *   method = method of the structure request
 *   Function: verify if the method is valid in req->method
 *              in case of error, set the *err and return the error number
 */
static int is_not_method_allowed(char *method, int *err)
{
    if (strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0)
    {
        return 0;
    }
    *err = METHOD_NOT_ALLOWED;
    return METHOD_NOT_ALLOWED;
}

/*
 *   request = request string to parse
 *   size = length of the request
 *   Function: parse a request string and
 *             return a struct request fullfilled, in case of error
 *              sets the *err pointer to the number of the error
 */
struct request *parser_request(char *raw_request, size_t size, int *err)
{
    struct request *req = sub_parser_request(raw_request, size);
    if (!req)
    {
        *err = REQUEST_ERR;
        return NULL;
    }

    if (is_not_method_allowed(req->method, err)
        || is_not_protocol_valid(req->version, err)
        || not_contain_host(req->hash_map->data[0], err))
    {
        free_request(req);
        return NULL;
    }
    return req;
}

#if defined(CUSTOM_MAIN) && defined(MAIN1)
int main(void)
{
    char req_string[] = "GET /path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host:\0v\0\r\n"
                        "\r\n"
                        "this \0is t\0\0he body";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    if (req && err == 0)
    {
        printf("%s\n", req->method);
        printf("%s\n", req->target);
        printf("%s\n", req->version);
        for (size_t i = 0; i < req->body_size; ++i)
        {
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
    if (!req && err != 0)
        printf("there is an error of parsing\n");
    return 0;
}
#endif /* CUSTOM_MAIN */
