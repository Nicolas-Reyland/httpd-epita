#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define IPADDR "127.5.5.5"
#define PORT 42069

static void slowloris(int socket_fd, int client_fd);

int main(void)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    char buffer[4096] = { 0 };
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, IPADDR, &serv_addr.sin_addr);

    int client_fd =
        connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    slowloris(socket_fd, client_fd);

    close(client_fd);
    close(socket_fd);

    return 0;
}

static bool set_socket_nonblocking_mode(int socket_fd);

void slowloris(int socket_fd, int client_fd)
{
    srand(time(NULL));
    set_socket_nonblocking_mode(socket_fd);
    char buffer[4096];
    char msg[] = "GET / HTTP/1.1\r\nHost: " IPADDR "\r\n\r\n";
    size_t msg_size = sizeof(msg) - 1;
    size_t remaining_to_write = msg_size;
    while (remaining_to_write)
    {
        // right size to write
        size_t write_size = 1 + rand() % 20;
        if (write_size > remaining_to_write)
            write_size = remaining_to_write;

        printf("Writing %zu bytes\n", write_size);

        write(socket_fd, msg + msg_size - remaining_to_write, write_size);
        remaining_to_write -= write_size;

        sleep(1);
    }
    printf("Done writing\n");

    ssize_t num_read = read(client_fd, buffer, 4095);
    if (num_read != -1)
    {
        buffer[num_read] = 0;
        printf("Got :\n'''\n%s\n'''\n", buffer);
    }
    else
    {
        printf("Got %s\n", strerror(errno));
    }
}

bool set_socket_nonblocking_mode(int socket_fd)
{
    int flags;

    if ((flags = fcntl(socket_fd, F_GETFL, 0)) == -1)
    {
        return false;
    }

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1)
    {
        return false;
    }

    return true;
}
