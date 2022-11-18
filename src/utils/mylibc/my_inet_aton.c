#include "my_inet_aton.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/socket.h>

static void sub_c_zero(char *c, const char **cp, int *base);

static int sub_big_switch(int n, unsigned int parts[4], in_addr_t *val_ptr);

int my_inet_aton(const char *cp, struct in_addr *addr)
{
    in_addr_t val;
    int base;
    int n;
    char c;
    unsigned int parts[4];
    unsigned int *pp = parts;
    c = *cp;

    while (1)
    {
        if (!isdigit(c))
            return 0;
        val = 0;
        base = 10;
        if (c == '0')
        {
            sub_c_zero(&c, &cp, &base);
        }
        while (1)
        {
            if (isascii(c) && isdigit(c))
            {
                val = (val * base) + (c - '0');
                c = *++cp;
            }
            else if (base == 16 && isascii(c) && isxdigit(c))
            {
                val = (val << 4) | (c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            }
            else
                break;
        }
        if (c == '.')
        {
            if (pp >= parts + 3)
                return 0;
            *pp++ = val;
            c = *++cp;
        }
        else
            break;
    }
    if (c != '\0' && (!isascii(c) || !isspace(c)))
        return 0;
    n = pp - parts + 1;
    if (sub_big_switch(n, parts, &val) == 0)
        return 0;

    if (addr)
        addr->s_addr = htonl(val);
    return 1;
}

void sub_c_zero(char *c, const char **cp, int *base)
{
    (*c) = *++(*cp);
    if (*c == 'x' || *c == 'X')
        *base = 16, *c = *++(*cp);
    else
        *base = 8;
}

int sub_big_switch(int n, unsigned int parts[4], in_addr_t *val_ptr)
{
    in_addr_t val = *val_ptr;
    switch (n)
    {
    case 0:
        return 0;
    case 1:
        break;
    case 2:
        if ((val > 0xffffff) || (parts[0] > 0xff))
            return 0;
        val |= parts[0] << 24;
        break;
    case 3:
        if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
            return 0;
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;
    case 4:
        if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff)
            || (parts[2] > 0xff))
            return 0;
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    }
    *val_ptr = val;
    return -1;
}
