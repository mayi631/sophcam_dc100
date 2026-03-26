#ifndef __CRC_H__
#define __CRC_H__

#include <stdint.h>
#include <stddef.h>

unsigned long update_crc(unsigned long crc, unsigned char *buf,
						int32_t len);

#endif /* __CRC_H__ */
