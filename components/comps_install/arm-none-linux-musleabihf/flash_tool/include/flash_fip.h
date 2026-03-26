#ifndef __FIP_H__
#define __FIP_H__

#define FIP_IMAGE_HEAD "FIPH" /* FIP Image Header 1 */
#define FIP_IMAGE_BODY "FIPB" /* FIP Image body */
#define FIP_MAGIC_NUMBER "CVBL01\n\0"
#define SPI_NAND_VERSION (0x1823a001)
#define BIT(nr) (1UL << (nr))

#define FLAGS_SET_PLANE_BIT (BIT(0))
#define FLAGS_SET_QE_BIT (BIT(1))
#define FLAGS_ENABLE_X2_BIT (BIT(2))
#define FLAGS_ENABLE_X4_BIT (BIT(3))
#define FLAGS_OW_SETTING_BIT (BIT(4))

#define BBP_LAST_PAGE 0x01
#define BBP_FIRST_PAGE 0x02
#define BBP_FIRST_2_PAGE 0x03

#define NUM_OF_NAND_FIP_BACKUP (2)
#define BAK_BLOCK_COUNT 10

typedef struct _spi_nand_info_t {
    uint32_t version;
    uint32_t id;
    uint32_t page_size;

    uint32_t spare_size;
    uint32_t block_size;
    uint32_t pages_per_block;

    uint32_t fip_block_cnt;
    unsigned char pages_per_block_shift;
    unsigned char badblock_pos;
    unsigned char dummy_data1[2];
    uint32_t flags;
    unsigned char ecc_en_feature_offset;
    unsigned char ecc_en_mask;
    unsigned char ecc_status_offset;
    unsigned char ecc_status_mask;
    unsigned char ecc_status_shift;
    unsigned char ecc_status_uncorr_val;
    unsigned char dummy_data2[2];
    uint32_t erase_count; // erase count for sys base block
    unsigned char sck_l;
    unsigned char sck_h;
    unsigned short max_freq;
    uint32_t sample_param;
    unsigned char xtal_switch;
    unsigned char dummy_data3[71];
} _spi_nand_info_t;

typedef struct block_header_t {
    unsigned char tag[4];
    uint32_t bc_or_seq;
    uint32_t checknum;
    uint32_t dummy_2;
} block_header_t;

typedef struct _fip_param1_t {
    unsigned long magic1; /* fip magic number*/
    uint32_t magic2;
    uint32_t param_cksum;
    struct _spi_nand_info_t nand_info;
} _fip_param1_t;


/* callback for nand flash write fip */
ERROR_TYPE nand_flash_write_fip(hal_flash *flash, uint8_t *input_data,
                                        uint32_t write_len);
/* callback for nor flash write fip */
ERROR_TYPE nor_flash_write_fip(hal_flash *flash, uint8_t **input_data,
                                       uint32_t write_len);

#endif	// __FLASH_H__


