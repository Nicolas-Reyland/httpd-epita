#include "utils/response/tools_response/tools_response.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "process/sig_handlers.h"
#include "utils/logging.h"
#include "utils/response/response.h"

#define READ_BUFF_SIZE 4096
#define _XOPEN_SOURCE_EXTENDED 1

int isDir(const char *fileName)
{
    struct stat path;

    stat(fileName, &path);

    return S_ISDIR(path.st_mode);
}

int is_path_traversal_attack(char *path, struct vhost *vhost)
{
    char *root_dir = hash_map_get(vhost->map, "root_dir");
    char *resolved_path = malloc(PATH_MAX);
    resolved_path = realpath(path, resolved_path);

    if (strncmp(root_dir, path, strlen(root_dir)) == 0)
    {
        free(resolved_path);
        return 0;
    }
    free(resolved_path);
    return 1;
}

/*
 *   target = ressource that the client want to get
 *   vhost = vhost that the server gave to extract the root dir
 *   Function: return the full path of the ressource concat with
 *             the root dir from vhost
 */
char *get_path_ressource(char *target, struct vhost *vhost)
{
    char *root_dir = hash_map_get(vhost->map, "root_dir");
    if (!root_dir)
    {
        return NULL;
    }
    char *path = malloc(strlen(root_dir) + 1);
    path = strcpy(path, root_dir);
    path = realloc(path, strlen(path) + strlen(target) + 1);
    path = strcat(path, target);

    // check if path is a directory and if it is, add the default file to the
    // path
    int is_dir = isDir(path);
    if (is_dir != 0)
    {
        char *default_file = hash_map_get(vhost->map, "default_file");
        if (!default_file)
        {
            // TODO: MEMORY LEAKS
            log_error("Default file doesn t exist in Vhost config\n");
            free(path);
            graceful_shutdown();
        }
        path = realloc(path, strlen(path) + strlen(default_file) + 1);
        path = strcat(path, default_file);
        return path;
    }
    else
    {
        return path;
    }
}

/*
 *   path = the full path of the ressource
 *   vhost = the structure response w/ headers
 *   Function: concatain the response with the contain of the file given in
 * parameter and return the response
 */
char *put_ressource_resp(char *path, size_t *size, struct vhost *vhost,
                         size_t *err)
{
    // if we can open the file:
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

    if (is_path_traversal_attack(path, vhost) == 1)
    {
        *err = 403;
        return NULL;
    }

    FILE *file = fopen(path, "r");
    char buff[READ_BUFF_SIZE];
    //--------puts /r/n one more time between headers and ressources
    // realloc_and_concat(resp, "\r\n", false);
    //--------
    char *res = NULL;
    size_t nb_read;
    while ((nb_read = fread(buff, 1, READ_BUFF_SIZE, file)) != 0)
    {
        void *res_tmp = realloc(res, *size + nb_read);
        if (res_tmp == NULL)
        {
            free(res);
            *size = 0;
            fclose(file);
            *err = 403;
            log_error("%s: %s\n", __func__, strerror(errno));
            return NULL;
        }
        res = res_tmp;
        memcpy(res + *size, buff, nb_read);
        *size += nb_read;
    }
    if (ferror(file))
    {
        log_error("%s: an error occured while reading \"%s\"\n", __func__,
                  path);
        *size = 0;
        free(res);
        *err = 403;
        return NULL;
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
