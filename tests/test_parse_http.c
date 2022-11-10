#include <criterion/criterion.h>
#include <stdbool.h>

#include "utils/parsers/http/parser_request.h"

Test(ParseRequest, test_nbkeys)
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
    int nb_keys = 0;
    if (req && req->hash_map)
        nb_keys = req->hash_map->num_keys;
    free_request(req);
    cr_assert(nb_keys == 4);
}

Test(ParseRequest, perferct_request)
{
    char req_string[] =
        "HE\0AD     /p\0ath/script.cgi?field1=value1&field2=value2 "
        "   H\0\0\0TTP/1.1     \r\n"
        "con\0nexion: close     \r\n"
        "insh:     c\0amarche   \r\n"
        "key:   \0v\0\r\n"
        "Host:    \0v\0\r\n"
        "\r\n"
        "this \0is t\0\0he body";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(err == 0);
}

Test(ParseRequest, test_bodyNULL)
{
    char req_string[] = "GET /path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host:\0v\0\r\n"
                        "\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    char *body = req->body;
    free_request(req);
    cr_assert(body == NULL);
}

Test(ParseRequest, test_sizebody)
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
    int size_body = 0;
    if (req)
        size_body = req->body_size;
    free_request(req);
    cr_assert(size_body == 19);
}

Test(ParseRequest, test_no_CRLFCRLF)
{
    char req_string[] = "GET /path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host:\0v\0\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(req == NULL);
}

Test(ParseRequest, test_no_CRLFCRLF_err)
{
    char req_string[] = "GET /path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host:\0v\0\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(err == 400);
}

Test(ParseRequest, test_no_host)
{
    char req_string[] = "GET /path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(err == 400);
}

Test(ParseRequest, test_protocol_false)
{
    char req_string[] = "GET /path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1111111\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host:\0v\0\r\n"
                        "\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(err == 505);
}

Test(ParseRequest, test_protocol_method_false)
{
    char req_string[] = "PUT /path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host:\0v\0\r\n"
                        "\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(err == 405);
}

Test(ParseRequest, test_not_enough_header)
{
    char req_string[] = "/path/script.cgi?field1=value1&field2=value2 "
                        "H\0\0\0TTP/1.1\r\n"
                        "con\0nexion: close\r\n"
                        "insh: c\0amarche\r\n"
                        "key:\0v\0\r\n"
                        "Host:\0v\0\r\n"
                        "\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(err == 400);
}

Test(ParseRequest, test_invalid)
{
    char req_string[] = "\r\n";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    cr_assert_null(req, "Expected TO BE NULL");
    free_request(req);
    cr_assert(err == 400);
}

Test(ParseRequest, option_invalid)
{
    char req_string[] =
        "HE\0AD     /p\0ath/script.cgi?field1=value1&field2=value2 "
        "   H\0\0\0TTP/1.1     \r\n"
        "con\0nexion: close     \r\n"
        "insh:     c\0amarche   \r\n"
        "key:   \0v\0\r\n"
        "Host :    \0v\0\r\n"
        "\r\n"
        "this \0is t\0\0he body";
    size_t size = sizeof(req_string) - 1;
    int err = 0;
    struct request *req = parser_request(req_string, size, &err);
    free_request(req);
    cr_assert(err == 400);
}
