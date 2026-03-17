#ifndef FLASH_TOOL_H
#define FLASH_TOOL_H

/* flash_tool opcode */
enum {
    OP_WRITE = 0,
    OP_WRITE_PART = 1,
    OP_WRITE_FIP = 2,
    OP_READ = 3,
    OP_READ_PART = 4,
    OP_READ_CHIP = 5,
    OP_ERASE = 6,
    OP_ERASE_PART = 7,
    OP_ERASE_CHIP = 8,
    OP_DUMP_INFO = 9,
    OP_HELP = 10,
    OP_UNKNOWN = 11
};

#endif /* FLASH_TOOL_H */
