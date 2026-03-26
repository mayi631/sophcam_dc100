#ifndef FLASH_LOG_H
#define FLASH_LOG_H

#ifdef DEBUG
    #define debug(...) do { \
        printf("[%s] DEBUG: ", __FUNCTION__); \
        printf(__VA_ARGS__); \
    } while(0)
#else
    #define debug(...)
#endif

#define ERROR(...) do { \
    printf("[%s] ERROR: ", __FUNCTION__); \
    printf(__VA_ARGS__); \
} while(0)

#define WARN(...) do { \
    printf("[%s] WARNING: ", __FUNCTION__); \
    printf(__VA_ARGS__); \
} while(0)

#define INFO(...) do { \
    printf("[%s] INFO: ", __FUNCTION__); \
    printf(__VA_ARGS__); \
} while(0)

#endif /* FLASH_LOG_H */
