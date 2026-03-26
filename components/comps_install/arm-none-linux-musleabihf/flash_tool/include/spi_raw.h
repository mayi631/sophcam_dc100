#ifndef SPI_RAW_H
#define SPI_RAW_H
#include <stdint.h>
#include "flash_error.h"

ERROR_TYPE spi_raw_read(int32_t  dev,
                            uint64_t read_start,
                            uint32_t read_length,
                            uint8_t  *out_buf,
                            uint32_t erasesize,
                            uint32_t writesize,
                            uint32_t oobsize,
                            uint64_t dev_off,
                            uint32_t dev_size,
                            uint32_t *totalread,
                            bool     with_oob);

ERROR_TYPE spi_raw_erase(int32_t  dev,
                             uint32_t erasesize,
                             uint64_t erase_start,
                             uint32_t erase_length,
                             uint64_t dev_off,
                             uint32_t dev_size,
                             uint32_t *totalerase);

ERROR_TYPE spi_raw_write(int32_t  dev,
                             uint64_t write_start,
                             uint32_t write_length,
                             uint8_t  *input_buf,
                             uint32_t erasesize,
                             uint32_t writesize,
                             uint32_t oobsize,
                             uint64_t dev_off,
                             uint32_t dev_size,
                             uint32_t *totalwrite,
                             bool     with_oob);
#endif /* SPI_RAW_H */

