#ifndef __CVI_LOG_H__
#define __CVI_LOG_H__

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "cvi_datatype.h"

/* __linux__ 由工具链提供，可通过 xxx-gcc -dM -E - < /dev/null | grep linux 查看 */
#if defined(__linux__)
#include <string.h>
#include <syslog.h>
#include <sys/prctl.h>

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/* void openlog(const char *ident, int option, int facility);
 * openlog() 的使用是可选的，如果没有调用，它将在第一次调用 syslog() 时自动调用，在这种情况下，ident 将默认为 NULL。
 *
 * ident: 指向的字符串被添加到每条日志信息的前面，一般指定为程序名。如果 ident 为 NULL，则使用程序名称。
 * option: LOG_PID 在每条日志信息中添加进程 ID。
 *         LOG_CONS 如果无法将消息发送到系统日志守护进程，则将消息打印到系统控制台。
 * facility: 指定日志的类型，常用的有 LOG_USER（用户级消息）和 LOG_DAEMON（系统守护进程消息）。
 *
 * 使用 prctl 获取进程名，并传入 openlog，主要是因为 musl 中并没有默认使用进程名作为 ident。
 */
#define CVI_LOG_OPEN()                                                         \
    do {                                                                       \
        char proc_name[32] = {0};                                              \
        prctl(PR_GET_NAME, proc_name, 0, 0, 0);                                \
        openlog(proc_name, LOG_PID | LOG_CONS, LOG_USER);                      \
    } while(0)
#define CVI_LOG_CLOSE() closelog()

/*
 * 日志等级：
 * `LOG_EMERG`: system is unusable
 * `LOG_ALERT`: action must be taken immediately
 * `LOG_CRIT`: critical conditions
 * `LOG_ERR`: error conditions
 * `LOG_WARNING`: warning conditions
 * `LOG_NOTICE`: normal, but significant, condition
 * `LOG_INFO`: informational message
 * `LOG_DEBUG`: debug-level message
 */
#define CVI_LOGF(fmt, ...) syslog(LOG_EMERG, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define CVI_LOGA(fmt, ...) syslog(LOG_ALERT, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define CVI_LOGC(fmt, ...) syslog(LOG_CRIT, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define CVI_LOGE(fmt, ...) syslog(LOG_ERR, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define CVI_LOGW(fmt, ...) syslog(LOG_WARNING, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define CVI_LOGN(fmt, ...) syslog(LOG_NOTICE, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define CVI_LOGI(fmt, ...) syslog(LOG_INFO, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#ifdef DEBUG
#define CVI_LOGD(fmt, ...) syslog(LOG_DEBUG, "[%s:%d %s] " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define CVI_LOGD(fmt, ...)
#endif

#else /* 针对非linux的 rtos 环境 */

/* debug log level define */
#define CVI_LOG_DEBUG        (8)    // LOG_DEBUG
#define CVI_LOG_INFO         (7)    // LOG_INFO
#define CVI_LOG_NOTICE       (6)    // LOG_NOTICE
#define CVI_LOG_WARNING      (5)    // LOG_WARNING
#define CVI_LOG_ERROR        (4)    // LOG_ERR
#define CVI_LOG_CRIT         (3)    // LOG_CRIT
#define CVI_LOG_ALERT        (2)    // LOG_ALERT
#define CVI_LOG_FATAL        (1)    // LOG_EMERG
#define CVI_LOG_NONE         (0)    // 0

/* default builtin log*/
#ifndef BUILT_LOG_LEVEL
#define BUILT_LOG_LEVEL CVI_LOG_INFO
#endif

/* If DEBUG is defined in the file, output DEBUG logs for this file */
#ifdef DEBUG
#undef BUILT_LOG_LEVEL
#define BUILT_LOG_LEVEL CVI_LOG_DEBUG
#endif

#define CVI_LOG(log_level, level_str, fmt, ...)                                         \
    do {                                                                                \
        if (BUILT_LOG_LEVEL >= log_level) {                                             \
            printf(level_str "[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);   \
        }                                                                               \
    } while (0)

#define CVI_LOGD(fmt, ...) CVI_LOG(CVI_LOG_DEBUG, "[DBG]", fmt, ##__VA_ARGS__)
#define CVI_LOGI(fmt, ...) CVI_LOG(CVI_LOG_INFO, "[INF]", fmt, ##__VA_ARGS__)
#define CVI_LOGN(fmt, ...) CVI_LOG(CVI_LOG_NOTICE, "[NOTICE]", fmt, ##__VA_ARGS__)
#define CVI_LOGW(fmt, ...) CVI_LOG(CVI_LOG_WARNING, "[WRN]", fmt, ##__VA_ARGS__)
#define CVI_LOGE(fmt, ...) CVI_LOG(CVI_LOG_ERROR, "[ERR]", fmt, ##__VA_ARGS__)
#define CVI_LOGC(fmt, ...) CVI_LOG(CVI_LOG_CRIT, "[CRIT]", fmt, ##__VA_ARGS__)
#define CVI_LOGA(fmt, ...) CVI_LOG(CVI_LOG_ALERT, "[ALERT]", fmt, ##__VA_ARGS__)
#define CVI_LOGF(fmt, ...) CVI_LOG(CVI_LOG_FATAL, "[EMERG]", fmt, ##__VA_ARGS__)
#endif

#ifndef CVI_LOG_ASSERT
#define CVI_LOG_ASSERT(x, ...)      \
    do {                            \
        if (!(x)) {                 \
            CVI_LOGE(__VA_ARGS__);  \
            abort();                \
        }                           \
    } while (0)
#endif

#endif //__CVI_LOG_H__
