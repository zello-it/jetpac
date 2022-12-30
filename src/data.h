#include <stdint.h>

typedef uint8_t byte;
typedef int8_t sbyte;

// utils
sbyte abs(sbyte b) {
    return(b < 0 ? -b : b);
}
sbyte sgn(sbyte b) {
    return (b > 0) - (0 > b);
}

