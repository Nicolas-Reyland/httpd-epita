#include "my_hton.h"

static uint16_t swap_uint16(uint16_t val);

static uint32_t swap_uint32(uint32_t val);

uint16_t my_htons(uint16_t x)
{
#if BYTE_ORDER != BIG_ENDIAN
    return x;
#else /* BYTE_ORDER != BIG_ENDIAN */
    return swap_uint16(x);
#endif /* BYTE_ORDER != BIG_ENDIAN */
}

uint32_t my_htonl(uint32_t x)
{
#if BYTE_ORDER != BIG_ENDIAN
    return x;
#else /* BYTE_ORDER != BIG_ENDIAN */
    return swap_uint32(x);
#endif /* BYTE_ORDER != BIG_ENDIAN */
}

uint16_t swap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xff00ff00) | ((val >> 8) & 0xff00ff);
    return (val << 16) | (val >> 16);
}
