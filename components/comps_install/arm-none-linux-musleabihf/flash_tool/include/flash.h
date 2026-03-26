#ifndef __FLASH_H__
#define __FLASH_H__
#include <stdbool.h>
#include "flash_error.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
#define MTD_ABSENT		 0
#define MTD_RAM			 1
#define MTD_ROM			 2
#define MTD_SPIFLASH	 3
#define MTD_NANDFLASH	 4 /* SLC NAND */
#define MTD_DATAFLASH	 6
#define MTD_UBIVOLUME	 7
#define MTD_MLCNANDFLASH 8 /* MLC NAND (including TLC) */

#define MAX_PARTS      32 /* Flash max partition number */
#define FLASH_NAME_LEN 32 /** Flash Name max length */
#define DEV_MTDBASE    "/dev/mtd"
#define PROC_MTD_FILE  "/proc/mtd"
#define FLASH_PATH_MAX       128

/* r/w flag */
#define FLASH_RW_FLAG_RAW      1
#define FLASH_RW_FLAG_WITH_OOB 2

/* flash type */
typedef enum FLASH_TYPE_E {
    FLASH_TYPE_SPI_0,  /**< SPI flash type */
    FLASH_TYPE_NAND_0, /**< NAND flash type */
    FLASH_TYPE_EMMC_0, /**< eMMC flash type */
    FLASH_TYPE_BUTT    /**< Invalid flash type */
} FLASH_TYPE_E;

/* Flash partition descriptions */
typedef struct flash_partinfo {
    uint64_t offset;                    /**< Partiton start address */
    uint32_t size;                      /**< Partition size */
    uint32_t erasesize;                 /**< The erase size of the flash where this partition at */
    char     devname[FLASH_NAME_LEN];   /**< The device node name where this partition relate to */
    char     partname[FLASH_NAME_LEN];  /**< The partition name of this partition */
} flash_partinfo;

/* Flash operation descriptions */
typedef struct flash_ops {
    ERROR_TYPE (*raw_read) (int32_t dev,
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
    ERROR_TYPE (*raw_write) (int32_t dev,
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
    ERROR_TYPE (*raw_erase) (int32_t dev,
                                 uint32_t erasesize,
                                 uint64_t erase_start,
                                 uint32_t erase_length,
                                 uint64_t dev_off,
                                 uint32_t dev_size,
                                 uint32_t *totalerase);
} flash_ops;

/* Flash Information */
typedef struct hal_flash {
    uint32_t totalsize;             /**< flash total size */
    uint32_t erasesize;             /**< flash erase size */
    uint32_t writesize;             /**< flash r/w size */
    uint32_t oobsize;               /**< flash OOB size */
    uint8_t  part_count;            /**< flash part count */
    FLASH_TYPE_E type;          /**< flash type */
    flash_ops *ops;             /**< operation callbacks on this flash */
    flash_partinfo *part_info;  /**< parition descriptions on this flash */
} hal_flash;

/*-------------------------- Structure Definition end ------------------------*/

/******************************* API declaration ******************************/

/* init hal_flash */
hal_flash* flash_init(void);

/* destroy hal_flash */
ERROR_TYPE flash_destroy(hal_flash *flash);

/* erase op */
ERROR_TYPE flash_erase(hal_flash *flash, uint64_t offset, uint32_t size,
                               uint32_t *totalerase);
/* read op */
ERROR_TYPE flash_read(hal_flash *flash, uint64_t offset, uint32_t size,
                              uint8_t *out_buf, uint32_t *totalread,
                              uint32_t flag);
/* write op */
ERROR_TYPE flash_write(hal_flash *flash, uint64_t offset, uint32_t size,
                               uint8_t *input_data, uint32_t *totalwrite,
                               uint32_t flag);

/* -------------------------- API declaration end --------------------------- */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __FLASH_H__ */

