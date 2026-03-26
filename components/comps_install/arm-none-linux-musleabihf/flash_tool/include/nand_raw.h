#ifndef NAND_RAW_H
#define NAND_RAW_H

#include <stdbool.h>
#include <sys/types.h>
#include "flash_error.h"

/**
 * MTD operation modes
 *
 * @MTD_OPS_PLACE_OOB:    OOB data are placed at the given offset (default)
 * @MTD_OPS_AUTO_OOB:    OOB data are automatically placed at the free areas
 *                        which are defined by the internal ecclayout
 * @MTD_OPS_RAW:        data are transferred as-is, with no error correction;
 *                        this mode implies %MTD_OPS_PLACE_OOB
 *
 * These modes can be passed to ioctl(MEMWRITE) and are also used internally.
 * See notes on "MTD file modes" for discussion on %MTD_OPS_RAW vs.
 * %MTD_FILE_MODE_RAW.
 */
enum {
    MTD_OPS_PLACE_OOB = 0,
    MTD_OPS_AUTO_OOB = 1,
    MTD_OPS_RAW = 2
};

typedef struct mtd_info_user {
    uint8_t  type;
    uint32_t flags;
    uint32_t size;
    uint32_t erasesize;
    uint32_t writesize;
    uint32_t oobsize;
    uint32_t ecctype;
    uint32_t eccsize;
} mtd_info_user;
#define MEMGETINFO _IOR('M', 1, struct mtd_info_user)

struct erase_info_user64 {
    uint64_t start;
    uint64_t length;
};
#define MEMERASE64 _IOW('M', 20, struct erase_info_user64)

struct mtd_oob_buf {
    uint32_t start;
    uint32_t length;
    uint8_t  *ptr;
};
#define MEMREADOOB _IOWR('M', 4, struct mtd_oob_buf)

/**
 * struct mtd_write_req - data structure for requesting a write operation
 *
 * @start:        start address
 * @len:        length of data buffer
 * @ooblen:        length of OOB buffer
 * @usr_data:    user-provided data buffer
 * @usr_oob:    user-provided OOB buffer
 * @mode:        MTD mode (see "MTD operation modes")
 * @padding:    reserved, must be set to 0
 *
 * This structure supports ioctl(MEMWRITE) operations, allowing data and/or OOB
 * writes in various modes. To write to OOB-only, set @usr_data == NULL, and to
 * write data-only, set @usr_oob == NULL. However, setting both @usr_data and
 * @usr_oob to NULL is not allowed.
 */
struct mtd_write_req {
    uint64_t start;
    uint64_t len;
    uint64_t ooblen;
    uint64_t usr_data;
    uint64_t usr_oob;
    uint8_t  mode;
    uint8_t  padding[7];
};
#define MEMWRITE _IOWR('M', 24, struct mtd_write_req)

#define MEMGETBADBLOCK _IOW('M', 11, loff_t)

#define MEMSETBADBLOCK _IOW('M', 12, loff_t)

ERROR_TYPE nand_raw_read(int32_t dev,
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
ERROR_TYPE nand_raw_write(int32_t dev,
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
ERROR_TYPE nand_raw_erase(int32_t dev,
                              uint32_t erasesize,
                              uint64_t erase_start,
                              uint32_t erase_length,
                              uint64_t dev_off,
                              uint32_t dev_size,
                              uint32_t *totalerase);

#endif /* NAND_RAW_H */
