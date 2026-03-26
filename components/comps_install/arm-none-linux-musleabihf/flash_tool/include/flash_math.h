#ifndef FLASH_MATH_H
#define FLASH_MATH_H

#define max(a, b) ((a) < (b) ? (b) : (a))
#define min(a, b) ((a) > (b) ? (b) : (a))

#define ALIGN(a, b) ((((a) - 1) / (b) + 1) * (b))
#define ALIGN_DOWN(a, b) ((a) / (b) * (b))

#endif /* FLASH_MATH_H*/
