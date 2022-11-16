#ifndef MY_INET_ATON_H
#define MY_INET_ATON_H

struct in_addr;

int my_inet_aton(const char *cp, struct in_addr *addr);

#endif /* !MY_INET_ATON_H */
