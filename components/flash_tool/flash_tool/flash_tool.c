#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <inttypes.h>

#include "flash_log.h"
#include "flash_error.h"
#include "flash.h"
#include "flash_fip.h"
#include "flash_math.h"
#include "flash_tool.h"

/*-------------------------- function declaration --------------------------*/
static uint8_t get_opcode(char *op);

static ERROR_TYPE op_write(hal_flash *flash, uint64_t offset,
                                   uint32_t size, char *input_file);

static ERROR_TYPE op_write_part(hal_flash *flash, char *partition_name,
                                        char *input_file);

static ERROR_TYPE op_write_fip(hal_flash *flash, char *input_file);

static ERROR_TYPE op_read(hal_flash *flash, uint64_t offset,
                                  uint32_t size, char *output_file);

static ERROR_TYPE op_read_part(hal_flash *flash, char* partition_name,
                                       char *output_file);

static ERROR_TYPE op_read_chip(hal_flash *flash, char *output_file);

static ERROR_TYPE op_erase(hal_flash *flash, uint64_t offset,
                                   uint32_t size);

static ERROR_TYPE op_erase_part(hal_flash *flash, char *partition_name);

static ERROR_TYPE op_erase_chip(hal_flash *flash);

static ERROR_TYPE op_dump_info(hal_flash *flash);
/*--------------------------------------------------------------------------*/

/* 目前支持nor和nand, 假设mtd设备存在, 未处理不存在情况 */
int32_t main(int32_t argc, char *argv[])
{
    hal_flash *flash = NULL;
    uint8_t opcode = OP_UNKNOWN;
    uint32_t size = 0;
    uint64_t offset = 0;
    char *op = NULL;
    char *input_file = NULL, *partition_name = NULL, *output_file = NULL;
    bool check_mark = false;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Parse command line arguments */
    if (argc < 2) {
        goto help;
    }

    /* get op code */
    op = argv[1];
    opcode = get_opcode(op);
    if (opcode == OP_HELP) {
        goto help;
    }

    for (int32_t i = 2; i < argc; i++) {
        if (strncmp(argv[i], "-i", 2) == 0) {
            if (i + 1 < argc) {
                input_file = argv[i + 1]; /* input file */
                i++;
            } else {
                ERROR("Missing value for -i\n");
                goto help;
            }
            continue;
        } else if (strncmp(argv[i], "-off", 4) == 0) {
            if (i + 1 < argc) {
                offset = strtoull(argv[i + 1], NULL, 16); /* offset */
                i++;
            } else {
                ERROR("Missing value for -off\n");
                goto help;
            }
            continue;
        } else if (strncmp(argv[i], "-s", 2) == 0) {
            if (i + 1 < argc) {
                size = strtoul(argv[i + 1], NULL, 16); /* size */
                i++;
            } else {
                ERROR("Missing value for -s\n");
                goto help;
            }
            continue;
        } else if (strncmp(argv[i], "-o", 2) == 0) {
            if (i + 1 < argc) {
                output_file = argv[i + 1]; /* output file */
                i++;
            } else {
                ERROR("Missing value for -o\n");
                goto help;
            }
            continue;
        } else if (strncmp(argv[i], "-p", 2) == 0) {
            if (i + 1 < argc) {
                partition_name = argv[i + 1]; /* partition name */
                i++;
            } else {
                ERROR("Missing value for -p\n");
                goto help;
            }
            continue;
        } else if (strncmp(argv[i], "-c", 2) == 0) {
            check_mark = true; /* verity flag */
            continue;
        } else {
            ERROR("Unknown parameter: %s\n", argv[i]);
            goto help;
        }
    }

    /* Print arguments */
    INFO("Flash op: %s\n", op ? op : "(null)");
    if (check_mark) {
        INFO("check mark\n");
    } else {
        INFO("don't check mark\n");
    }

    /* Init hal_flash */
    flash = flash_init();
    if (flash == NULL) {
        ERROR("init flash fail !\n");
        check_ret = FLASH_UNKNOWN_ERROR;
        goto out;
    }

    /* Exec flash op function */
    debug("opcode: %d\n", opcode);

    switch (opcode) {
        case OP_WRITE:
            check_ret = op_write(flash, offset, size, input_file);
            break;
        case OP_WRITE_PART:
            check_ret = op_write_part(flash, partition_name, input_file);
            break;
        case OP_WRITE_FIP:
            check_ret = op_write_fip(flash, input_file);
            break;
        case OP_READ:
            check_ret = op_read(flash, offset, size, output_file);
            break;
        case OP_READ_PART:
            check_ret = op_read_part(flash, partition_name, output_file);
            break;
        case OP_READ_CHIP:
            check_ret = op_read_chip(flash, output_file);
            break;
        case OP_ERASE:
            check_ret = op_erase(flash, offset, size);
            break;
        case OP_ERASE_PART:
            check_ret = op_erase_part(flash, partition_name);
            break;
        case OP_ERASE_CHIP:
            check_ret = op_erase_chip(flash);
            break;
        case OP_DUMP_INFO:
            check_ret = op_dump_info(flash);
            break;
        /* TODO we need it ? */
        case OP_HELP:
            goto help;
        default:
            ERROR("Unknown flash op !\n");
            check_ret =  FLASH_PARAM_ERROR;
    }

    if(check_ret != FLASH_OK) {
        ERROR("Exec flash op \"%s\" fail !\n", op);
        goto free;
    }

free:
    /* flash close */
    if (flash != NULL) {
        check_ret = flash_destroy(flash);
        if (check_ret != FLASH_OK) {
            ERROR("destroy flash fail !\n");
        } else {
            flash = NULL;
            INFO("close flash success !\n");
        }
    }
out:
    return check_ret;

help:
    printf("\nflash_tool cmd list:\n\n");

    printf("1.generic:\n");
    printf("    flash_tool help\n");
    printf("    flash_tool info\n\n");

    printf("2.write:\n");
    printf("    flash_tool write -i <input file name> "
           "-off <offset> -s <size> [-c]\n");
    printf("        warning: need erase before use it !\n");
    printf("    flash_tool write.part -i <input file name> "
           "-p <partition name> [-c]\n");
    printf("    flash_tool write.fip -i <input file name> [-c]\n\n");

    printf("3.read:\n");
    printf("    flash_tool read -o <output file name> "
           "-off <offset> -s <size> [-c]\n");
    printf("    flash_tool read.part -o <output file name> "
           "-p <partition name> [-c]\n");
    printf("    flash_tool read.chip -o <output file name> [-c]\n\n");

    printf("4.erase:\n");
    printf("    flash_tool erase -off <offset> -s <size>\n");
    printf("    flash_tool erase.part -p <partition name>\n");
    printf("    flash_tool erase.chip\n\n");

    printf("Parameter parsing: \n"
           "    -i   <input file name> : Enter the file name, "
           "usually saving the flash content to be updated \n"
           "    -o   <output file name>: Enter the file name, "
           "usually saving the content read from flash \n"
           "    -off <offset>          : Start address of a flash operation \n"
           "    -s   <size>            : Size of a flash operation \n"
           "    -p   <partition name>  : Partition names in flash \n"
           "    -c                     : Is it necessary to verify \n\n");
    exit(0);
}

/* <util function> char *op translate into uint8_t opcode */
static uint8_t get_opcode(char *op)
{
    if(op == NULL) {
        ERROR("op is null !\n");
        return OP_UNKNOWN;
    }

    if        (strcmp(op, "write"     ) == 0) {
        return OP_WRITE;
    } else if (strcmp(op, "write.part") == 0) {
        return OP_WRITE_PART;
    } else if (strcmp(op, "write.fip" ) == 0) {
        return OP_WRITE_FIP;
    } else if (strcmp(op, "read"      ) == 0) {
        return OP_READ;
    } else if (strcmp(op, "read.part" ) == 0) {
        return OP_READ_PART;
    } else if (strcmp(op, "read.chip" ) == 0) {
        return OP_READ_CHIP;
    } else if (strcmp(op, "erase"     ) == 0) {
        return OP_ERASE;
    } else if (strcmp(op, "erase.part") == 0) {
        return OP_ERASE_PART;
    } else if (strcmp(op, "erase.chip") == 0) {
        return OP_ERASE_CHIP;
    } else if (strcmp(op, "info"      ) == 0) {
        return OP_DUMP_INFO;
    } else if (strcmp(op, "help"      ) == 0) {
        return OP_HELP;
    } else {
        WARN("Unknown op !\n");
    }

    return OP_UNKNOWN;
}

static ERROR_TYPE op_erase(hal_flash *flash, uint64_t offset,
                                   uint32_t size)
{
    uint32_t real_erase;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    INFO("offset: %#"PRIx64"\n", offset);
    INFO("size: %#"PRIx32"\n", size);

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (offset % flash->erasesize != 0 || size % flash->erasesize != 0) {
        ERROR("offset and size should align with erasesize(%#"PRIx32")\n",
              flash->erasesize);
        INFO("You can adjust offset to %#"PRIx64" and size to %#"PRIx32"\n",
             ALIGN_DOWN(offset, flash->erasesize),
             ALIGN(size, flash->erasesize));
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* Erase */
    debug("Erase %#"PRIx32" byte from %#"PRIx64"...\n", size, offset);

    check_ret = flash_erase(flash, offset, size, &real_erase);
    if (check_ret != FLASH_OK) {
        ERROR("Erase flash fail\n");
        goto out;
    }
    INFO("Erase flash success ! erase_size: %#"PRIx32"\n", real_erase);
    printf("\n");

out:
    return check_ret;
}

static ERROR_TYPE op_erase_chip(hal_flash *flash)
{
    uint32_t real_erase;
    ERROR_TYPE check_ret = FLASH_OK;

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* Erase */
    INFO("Erase all (size: %#"PRIx32") ...\n", flash->totalsize);
    debug("Erase %#"PRIx32" byte from %#"PRIx64"...\n", flash->totalsize,
          (uint64_t)0x0);

    check_ret = flash_erase(flash, (uint64_t)0x0, flash->totalsize,
                                &real_erase);
    if (check_ret != FLASH_OK) {
        ERROR("Erase flash fail\n");
        goto out;
    }
    INFO("Erase flash success ! erase_size: %#"PRIx32"\n", real_erase);
    printf("\n");

out:
    return check_ret;
}

static ERROR_TYPE op_erase_part(hal_flash *flash, char *partition_name)
{
    uint8_t part_idx = 0;
    uint32_t part_size, real_erase;
    uint64_t part_offset;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    ERROR("partition_name: %s\n", partition_name ? partition_name : "(null)");

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (partition_name == NULL) {
        INFO("partition_name cannot null\n");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* Find partition index by partition name */
    for (uint8_t i = 0; i < MAX_PARTS; ++i) {
        if (strcmp(flash->part_info[i].partname, partition_name) == 0) {
            part_idx = i;
            break;
        }
    }

    part_offset = flash->part_info[part_idx].offset;
    part_size   = flash->part_info[part_idx].size;

    /* Erase */
    debug("Erase %#"PRIx32" byte from %#"PRIx64"...\n", part_size, part_offset);

    check_ret = flash_erase(flash, part_offset, part_size, &real_erase);
    if (check_ret != FLASH_OK) {
        ERROR("Erase flash fail\n");
        goto out;
    }
    INFO("Erase flash success ! erase_size: %#"PRIx32"\n", real_erase);
    printf("\n");

out:
    return check_ret;
}

/* Need erase before write */
static ERROR_TYPE op_write(hal_flash *flash, uint64_t offset,
                                   uint32_t size, char *input_file)
{
    uint8_t *input_data = NULL;
    uint32_t real_write;
    int32_t fd_input_file;
    ssize_t read_count;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    INFO("offset: %#"PRIx64"\n", offset);
    INFO("size: %#"PRIx32"\n", size);
    INFO("input_file: %s\n", input_file ? input_file : "(null)");

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (input_file == NULL) {
        ERROR("input_file or cannot null\n");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (offset % flash->writesize != 0 || size % flash->writesize != 0) {
        ERROR("offset and size should align with writesize(%#"PRIx32")\n",
              flash->writesize);
        INFO("You can adjust offset to %#"PRIx64" and size to %#"PRIx32"\n",
             ALIGN_DOWN(offset, flash->writesize),
             ALIGN(size, flash->writesize));
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* Read input file */
    fd_input_file = open(input_file, O_RDWR, 777);
    if (fd_input_file < 0) {
        ERROR("load [%s] failed\n", input_file);
        check_ret = FLASH_FS_ERROR;
        goto out;
    }

    input_data = (uint8_t *)malloc(size);
    if (input_data == NULL) {
        ERROR("malloc buff failed\n");
        check_ret = FLASH_MM_ERROR;
        goto out;
    }
    memset(input_data, 0xff, size);

    read_count = read(fd_input_file, input_data, size);
    close(fd_input_file);

    if (read_count < 0) {
        ERROR("read %s failed\n", input_file);
        check_ret = FLASH_IO_ERROR;
        goto free;
    }
    debug("read %#"PRIxMAX" byte from input file.\n", (intmax_t)read_count);

    /* Write */
    debug("Write %#"PRIx32" byte from %#"PRIx64"...\n", size, offset);

    check_ret = flash_write(flash, offset, size, input_data,
                                &real_write, FLASH_RW_FLAG_RAW);
    if (check_ret != FLASH_OK) {
        ERROR("write flash fail\n");
        goto free;
    } else {
        INFO("Write flash success ! write_size: %#"PRIx32"\n", real_write);
    }
    printf("\n");

free:
    /* Free input data buf */
    if (input_data != NULL)
        free(input_data);
out:
    return check_ret;
}

static ERROR_TYPE op_write_part(hal_flash *flash, char *partition_name,
                                        char *input_file)
{
    uint8_t part_idx = 0, *input_data = NULL;
    uint32_t part_size, real_erase, real_write;
    uint64_t part_offset;
    int32_t fd_input_file;
    ssize_t read_count;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    INFO("input_file: %s\n", input_file ? input_file : "(null)");
    INFO("partition_name: %s\n", partition_name ? partition_name : "(null)");

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if ((input_file == NULL) || (partition_name == NULL)) {
        ERROR("input_file or partition_name cannot null\n");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* Find partition index by partition name */
    for (uint8_t i = 0; i < MAX_PARTS; ++i) {
        if (strcmp(flash->part_info[i].partname, partition_name) == 0) {
            part_idx = i;
            break;
        }
    }

    part_offset = flash->part_info[part_idx].offset;
    part_size   = flash->part_info[part_idx].size;

    /* Read input file */
    fd_input_file = open(input_file, O_RDWR, 777);
    if (fd_input_file < 0) {
        ERROR("load [%s] failed\n", input_file);
        check_ret = FLASH_FS_ERROR;
        goto out;
    }

    input_data = (uint8_t *)malloc(part_size);
    if (input_data == NULL) {
        ERROR("malloc buff failed\n");
        check_ret = FLASH_MM_ERROR;
        goto out;
    }
    memset(input_data, 0xff, part_size);

    read_count = read(fd_input_file, input_data, part_size);
    close(fd_input_file);

    if (read_count < 0) {
        ERROR("read %s failed\n", input_file);
        check_ret = FLASH_IO_ERROR;
        goto free;
    }
    debug("read %#"PRIxMAX" byte from input file.\n", (intmax_t)read_count);

    /* Erase */
    debug("Erase %#"PRIx32" byte from %#"PRIx64"...\n", part_size, part_offset);

    check_ret = flash_erase(flash, part_offset, part_size, &real_erase);
    if (check_ret != FLASH_OK) {
        ERROR("Erase flash fail\n");
        goto free;
    } else {
        INFO("Erase flash success ! erase_size: %#"PRIx32"\n", real_erase);
    }
    printf("\n");

    /* Write */
    debug("Write %#"PRIx32" byte from %#"PRIx64"...\n", part_size, part_offset);

    check_ret = flash_write(flash, part_offset, part_size, input_data,
                                &real_write, FLASH_RW_FLAG_RAW);
    if (check_ret != FLASH_OK) {
        ERROR("write flash fail\n");
        goto free;
    } else {
        INFO("Write flash success ! write_size: %#"PRIx32"\n", real_write);
    }
    printf("\n");

free:
    /* Free input data buf */
    if (input_data != NULL)
        free(input_data);
out:
    return check_ret;
}

static ERROR_TYPE op_read(hal_flash *flash, uint64_t offset,
                                  uint32_t size, char *output_file)
{
    uint8_t *output_data = NULL;
    uint32_t real_read;
    int32_t fd_output_file;
    ssize_t write_count;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    INFO("output_file: %s\n", output_file ? output_file : "(null)");
    INFO("offset: %#"PRIx64"\n", offset);
    INFO("size: %#"PRIx32"\n", size);

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (output_file == NULL) {
        ERROR("output_file cannot null\n");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (offset % flash->writesize != 0 || size % flash->writesize != 0) {
        ERROR("offset and size should align with writesize(%#"PRIx32")\n",
              flash->writesize);
        INFO("You can adjust offset to %#"PRIx64" and size to %#"PRIx32"\n",
             ALIGN_DOWN(offset, flash->writesize),
             ALIGN(size, flash->writesize));
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* malloc buffer for output data */
    output_data = (uint8_t *)malloc(size);
    if (output_data == NULL) {
        ERROR("malloc buff failed\n");
        check_ret = FLASH_MM_ERROR;
        goto out;
    }

    /* Read flash */
    debug("Read %#"PRIx32" byte from %#"PRIx64"...\n", size, offset);

    check_ret = flash_read(flash, offset, size, output_data,
                               &real_read, FLASH_RW_FLAG_RAW);
    if (check_ret != FLASH_OK) {
        ERROR("read flash fail\n");
        goto free;
    } else {
        INFO("Read flash success ! read_size: %#"PRIx32"\n", real_read);
    }
    printf("\n");

    /* Write to out file */
    fd_output_file = open(output_file, O_RDWR, 777);
    if (fd_output_file < 0) {
        ERROR("load [%s] failed\n", output_file);
        check_ret = FLASH_FS_ERROR;
        goto free;
    }

    if (ftruncate(fd_output_file, 0) == -1) {
        WARN("Clear the input file content fail !\n");
    }

    write_count = write(fd_output_file, output_data, real_read);
    close(fd_output_file);

    if (write_count < 0) {
        ERROR("write %s failed\n", output_file);
        check_ret = FLASH_IO_ERROR;
        goto free;
    }
    debug("Write %#"PRIxMAX" byte to input file.\n", (intmax_t)write_count);

free:
    /* Free input data buf */
    if (output_data != NULL)
        free(output_data);
out:
    return check_ret;
}

static ERROR_TYPE op_read_chip(hal_flash *flash, char *output_file)
{
    uint8_t *output_data = NULL;
    uint32_t chip_size, real_read;
    int32_t fd_output_file;
    ssize_t write_count;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    INFO("output_file: %s\n", output_file ? output_file : "(null)");

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (output_file == NULL) {
        ERROR("output_file cannot null\n");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    chip_size = flash->totalsize;

    /* malloc buffer for output data */
    output_data = (uint8_t *)malloc(chip_size);
    if (output_data == NULL) {
        ERROR("malloc buff failed\n");
        check_ret = FLASH_MM_ERROR;
        goto out;
    }

    /* Read flash */
    INFO("Read all (size: %#"PRIx32") ...\n", chip_size);
    debug("Read %#"PRIx32" byte from %#"PRIx64"...\n", chip_size,
          (uint64_t)0x0);

    check_ret = flash_read(flash, (uint64_t)0x0, chip_size, output_data,
                               &real_read, FLASH_RW_FLAG_RAW);
    if (check_ret != FLASH_OK) {
        ERROR("read flash fail\n");
        goto free;
    } else {
        INFO("Read flash success ! read_size: %#"PRIx32"\n", real_read);
    }
    printf("\n");

    /* Write to out file */
    fd_output_file = open(output_file, O_RDWR, 777);
    if (fd_output_file < 0) {
        ERROR("load [%s] failed\n", output_file);
        check_ret = FLASH_FS_ERROR;
        goto free;
    }

    if (ftruncate(fd_output_file, 0) == -1) {
        WARN("Clear the input file content fail !\n");
    }

    write_count = write(fd_output_file, output_data, real_read);
    close(fd_output_file);

    if (write_count < 0) {
        ERROR("write %s failed\n", output_file);
        check_ret = FLASH_IO_ERROR;
        goto free;
    }
    debug("Write %#"PRIxMAX" byte to input file.\n", (intmax_t)write_count);

free:
    /* Free input data buf */
    if (output_data != NULL)
        free(output_data);
out:
    return check_ret;
}

static ERROR_TYPE op_read_part(hal_flash *flash, char* partition_name,
                                       char *output_file)
{
    uint8_t part_idx = 0, *output_data = NULL;
    uint32_t part_size, real_read;
    uint64_t part_offset;
    int32_t fd_output_file;
    ssize_t write_count;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    INFO("output_file: %s\n", output_file ? output_file : "(null)");
    INFO("partition_name: %s\n", partition_name ? partition_name : "(null)");

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if ((output_file == NULL) || (partition_name == NULL)) {
        ERROR("output_file or partition_name cannot null\n");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* Find partition index by partition name */
    for (uint8_t i = 0; i < MAX_PARTS; ++i) {
        if (strcmp(flash->part_info[i].partname, partition_name) == 0) {
            part_idx = i;
            break;
        }
    }

    part_offset = flash->part_info[part_idx].offset;
    part_size   = flash->part_info[part_idx].size;

    /* malloc buffer for output data */
    output_data = (uint8_t *)malloc(part_size);
    if (output_data == NULL) {
        ERROR("malloc buff failed\n");
        check_ret = FLASH_MM_ERROR;
        goto out;
    }

    /* Read flash */
    debug("Read %#"PRIx32" byte from %#"PRIx64"...\n", part_size, part_offset);

    check_ret = flash_read(flash, part_offset, part_size, output_data,
                               &real_read, FLASH_RW_FLAG_RAW);
    if (check_ret != FLASH_OK) {
        ERROR("read flash fail\n");
        goto free;
    } else {
        INFO("Read flash success ! read_size: %#"PRIx32"\n", real_read);
    }
    printf("\n");

    /* Write to out file */
    fd_output_file = open(output_file, O_RDWR, 777);
    if (fd_output_file < 0) {
        ERROR("load [%s] failed\n", output_file);
        check_ret = FLASH_FS_ERROR;
        goto free;
    }

    if (ftruncate(fd_output_file, 0) == -1) {
        WARN("Clear the input file content fail !\n");
    }

    write_count = write(fd_output_file, output_data, real_read);
    close(fd_output_file);

    if (write_count < 0) {
        ERROR("write %s failed\n", output_file);
        check_ret = FLASH_IO_ERROR;
        goto free;
    }
    debug("Write %#"PRIxMAX" byte to input file.\n", (intmax_t)write_count);

free:
    /* Free input data buf */
    if (output_data != NULL)
        free(output_data);
out:
    return check_ret;
}

static ERROR_TYPE op_write_fip(hal_flash *flash, char *input_file)
{
    uint8_t *input_data = NULL;
    int32_t fd_input_file = -1;
    ssize_t read_count, file_size;
    ERROR_TYPE check_ret = FLASH_OK;

    /* Print arguments */
    INFO("input_file: %s\n", input_file ? input_file : "(null)");

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    if (input_file == NULL) {
        ERROR("input_file cannot null\n");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    /* Read input file */
    fd_input_file = open(input_file, O_RDWR, 777);
    if (fd_input_file < 0) {
        ERROR("load [%s] failed\n", input_file);
        check_ret = FLASH_FS_ERROR;
        goto out;
    }

    /* get file size */
    file_size = lseek(fd_input_file, 0, SEEK_END);
    if (file_size == -1) {
        ERROR("Error seeking file\n");
        check_ret = FLASH_FS_ERROR;
        goto close;
    }

    input_data = (uint8_t *)malloc(file_size);
    if (input_data == NULL) {
        ERROR("malloc buff failed\n");
        check_ret = FLASH_MM_ERROR;
        goto close;
    }

    if (lseek(fd_input_file, 0, SEEK_SET) == -1) {
        ERROR("Error re-seeking file\n");
        check_ret = FLASH_FS_ERROR;
        goto free;
    }

    read_count = read(fd_input_file, input_data, file_size);
    if (read_count < 0) {
        ERROR("read %s failed\n", input_file);
        check_ret = FLASH_IO_ERROR;
        goto free;
    }
    debug("read %#"PRIxMAX" byte from input file.\n", (intmax_t)read_count);

    /* Write fip */
    /* TODO */
    if (flash->type == FLASH_TYPE_NAND_0) {
        check_ret = nand_flash_write_fip(flash, input_data, file_size);
    } else if (flash->type == FLASH_TYPE_SPI_0) {
        /* This function is subject to modification input_data. */
        check_ret = nor_flash_write_fip(flash, &input_data, file_size);
    }
    if (check_ret != FLASH_OK) {
        ERROR("write fip fail\n");
        goto free;
    } else {
        INFO("Write fip success !\n");
    }
    printf("\n");

free:
    /* Free input data buf */
    if (input_data != NULL)
        free(input_data);
    input_data = NULL;
close:
    if (fd_input_file != -1) {
        close(fd_input_file);
    }
    fd_input_file = -1;
out:
    return check_ret;
}

static ERROR_TYPE op_dump_info(hal_flash *flash)
{
    ERROR_TYPE check_ret = FLASH_OK;

    if (flash == NULL) {
        ERROR("flash is NULL !");
        check_ret = FLASH_PARAM_ERROR;
        goto out;
    }

    INFO("Flash info :\n");
    printf("flash->type: %d\n", flash->type);
    printf("    [0: nor, 1: nand, 2: emmc, 3: invalid]\n");
    printf("flash->totalsize: %#"PRIx32"\n", flash->totalsize);
    printf("flash->erasesize: %#"PRIx32"\n", flash->erasesize);
    printf("flash->writesize: %#"PRIx32"\n", flash->writesize);
    printf("flash->oobsize: %#"PRIx32"\n", flash->oobsize);
    printf("flash->part_count: %#"PRIx8"\n", flash->part_count);
    printf("flash->part_info\n");
    for (int i = 0; i < flash->part_count; ++i) {
        printf("    <<mtd%d>>  offset: %#"PRIx64"  partname: %s.\n", i,
              flash->part_info[i].offset, flash->part_info[i].partname);
    }

out:
    return check_ret;
}
