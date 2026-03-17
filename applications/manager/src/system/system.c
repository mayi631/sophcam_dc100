#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/reboot.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "system.h"
#include "appcomm.h"
#include "bootsound.h"
#include "hal_gsensor.h"

#define DEFAULT_RTC_DEVICE "/dev/rtc0"

int32_t SYSTEM_SetDateTime(const SYSTEM_TM_S* pstDateTime)
{
    APPCOMM_CHECK_POINTER(pstDateTime, APP_EINVAL);
    APPCOMM_CHECK_EXPR(1970 < pstDateTime->s32year, APP_EINVAL);
    APPCOMM_CHECK_EXPR(APPCOMM_CHECK_RANGE(pstDateTime->s32mon, 1, 12), APP_EINVAL);
    APPCOMM_CHECK_EXPR(APPCOMM_CHECK_RANGE(pstDateTime->s32mday, 1, 31), APP_EINVAL);
    APPCOMM_CHECK_EXPR(APPCOMM_CHECK_RANGE(pstDateTime->s32hour, 0, 23), APP_EINVAL);
    APPCOMM_CHECK_EXPR(APPCOMM_CHECK_RANGE(pstDateTime->s32min, 0, 59), APP_EINVAL);
    APPCOMM_CHECK_EXPR(APPCOMM_CHECK_RANGE(pstDateTime->s32sec, 0, 59), APP_EINVAL);
    /*1.set systime*/
    time_t t = 0L;
    struct tm stTime;
    memset(&stTime, 0, sizeof(struct tm));
    stTime.tm_year = pstDateTime->s32year - 1900;
    stTime.tm_mon = pstDateTime->s32mon - 1;
    stTime.tm_mday = pstDateTime->s32mday;
    stTime.tm_hour = pstDateTime->s32hour;
    stTime.tm_min = pstDateTime->s32min;
    stTime.tm_sec = pstDateTime->s32sec;
    t = mktime(&stTime);
    struct timeval stNowTime = {t, 0};
    struct timezone tz = {-480, -1};
    settimeofday(&stNowTime, &tz);
    #if defined(__arm__) && !defined(__USE_GNU)
    #else
    syscall(SYS_settimeofday, &stNowTime, &tz);
    #endif

    /*2.set rtc time*/
    int32_t s32Ret = -1;
    int32_t s32fd_rtc = -1;
    s32fd_rtc = open(DEFAULT_RTC_DEVICE, O_RDWR);

    if (s32fd_rtc < 0) {
        CVI_LOGE("open %s error:%s\n", DEFAULT_RTC_DEVICE, strerror(errno));
    } else {
        localtime_r(&t, &stTime);

        struct rtc_time rtctm;
        rtctm.tm_year = stTime.tm_year;
        rtctm.tm_mon = stTime.tm_mon;
        rtctm.tm_mday = stTime.tm_mday;
        rtctm.tm_hour = stTime.tm_hour;
        rtctm.tm_min = stTime.tm_min;
        rtctm.tm_sec = stTime.tm_sec;
        rtctm.tm_wday = stTime.tm_wday;
        s32Ret = ioctl(s32fd_rtc, RTC_SET_TIME, &rtctm);

        if (s32Ret < 0) {
            CVI_LOGE("ioctl set rtc time error:%s\n", strerror(errno));
        }

        close(s32fd_rtc);
    }

    memset(&tz, 0x0, sizeof(tz));
    gettimeofday(&stNowTime, &tz);
    CVI_LOGI("######## %s - %s:%d\n", ctime(&(stNowTime.tv_sec)), getenv("TZ"), tz.tz_minuteswest);
    return 0;
}

int32_t SYSTEM_GetRTCDateTime(SYSTEM_TM_S* pstDateTime)
{
    APPCOMM_CHECK_POINTER(pstDateTime, APP_EINVAL);
    int32_t s32fd_rtc = -1;
    s32fd_rtc = open(DEFAULT_RTC_DEVICE, O_RDWR);

    if (s32fd_rtc < 0) {
        CVI_LOGE("open %s error:%s\n", DEFAULT_RTC_DEVICE, strerror(errno));
    } else {
        struct rtc_time rtctm;

        if (ioctl(s32fd_rtc, RTC_RD_TIME, &rtctm) < 0) {
            CVI_LOGE("ioctl get rtc time error:%s\n", strerror(errno));
        } else {
            pstDateTime->s32year = rtctm.tm_year + 1900;
            pstDateTime->s32mon = rtctm.tm_mon + 1;
            pstDateTime->s32mday = rtctm.tm_mday;
            pstDateTime->s32hour = rtctm.tm_hour;
            pstDateTime->s32min = rtctm.tm_min;
            pstDateTime->s32sec = rtctm.tm_sec;
        }
        close(s32fd_rtc);
    }

    return 0;
}

int32_t SYSTEM_SetDefaultDateTime(void)
{
    int32_t s32Ret = 0;
    SYSTEM_TM_S stDateTime = {0};
return 0;
    s32Ret = SYSTEM_GetRTCDateTime(&stDateTime);
    if (s32Ret != 0) {
        CVI_LOGE("SYSTEM_GetRTCDateTime get failed!\n");
        return -1;
    }
    /*set systime*/
    time_t t = 0L;
    struct tm stTime;
    memset(&stTime, 0, sizeof(struct tm));
    stTime.tm_year = stDateTime.s32year- 1900;
    stTime.tm_mon = stDateTime.s32mon - 1;
    stTime.tm_mday = stDateTime.s32mday;
    stTime.tm_hour = stDateTime.s32hour;
    stTime.tm_min = stDateTime.s32min;
    stTime.tm_sec = stDateTime.s32sec;
    t = mktime(&stTime);
    struct timeval stNowTime = {t, 0};
    struct timezone tz = {-480, -1};
    settimeofday(&stNowTime, &tz);
    #if defined(__arm__) && !defined(__USE_GNU)
    #else
    syscall(SYS_settimeofday, &stNowTime, &tz);
    #endif

    if (stDateTime.s32year < DATETIME_DEFAULT_YEAR) {
        stDateTime.s32year = DATETIME_DEFAULT_YEAR;
        stDateTime.s32mon = DATETIME_DEFAULT_MONTH;
        stDateTime.s32mday = DATETIME_DEFAULT_DAY;
        stDateTime.s32hour = DATETIME_DEFAULT_HOUR;
        stDateTime.s32min = DATETIME_DEFAULT_MINUTE;
        stDateTime.s32sec = DATETIME_DEFAULT_SECOND;
        s32Ret = SYSTEM_SetDateTime(&stDateTime);
        if (s32Ret != 0) {
            CVI_LOGE("SYSTEM_GetRTCDateTime get failed!\n");
            return -1;
        }
    }

    return 0;
}

void SYSTEM_Reboot(int32_t need_sync)
{
    if(need_sync)
        sync();
    reboot(RB_AUTOBOOT);
}

int32_t SYSTEM_SetGpioWakeup(HAL_GPIO_NUM_E gpio, uint32_t value)
{
    off_t offset;
    void *map_base;
    void *gpio_addr = NULL;
    int32_t devgpio_fd = -1;

    devgpio_fd = open("/dev/mem", O_RDWR | O_NDELAY);
    if (devgpio_fd < 0) {
        CVI_LOGE("open /dev/mem/ failed !\n");
        return -1;
    }

    offset = PWR_WAKEUP0_PIN_MEM & ~(sysconf(_SC_PAGE_SIZE) - 1);
    map_base = (void *)mmap(NULL, 4 + PWR_WAKEUP0_PIN_MEM - offset, PROT_READ | PROT_WRITE, MAP_SHARED, devgpio_fd, offset);
    if (map_base == (void *)(-1)) {
        CVI_LOGD("mmap failed \n");
        CVI_LOGD("errno = %d str = %s\n", errno, strerror(errno));
        close(devgpio_fd);
        return -1;
    }

    gpio_addr = map_base + PWR_WAKEUP0_PIN_MEM - offset;
    *(uint32_t *)gpio_addr = value;
    CVI_LOGD("*gpio_addr == %d \n", *(uint32_t *)gpio_addr);
    close(devgpio_fd);

    munmap((void*)gpio_addr, 4 + PWR_WAKEUP0_PIN_MEM - offset);
    return 0;
}

int32_t SYSTEM_GetStartupWakeupSource(SYSTEM_STARTUP_SRC_E* penStartupSrc)
{
    APPCOMM_CHECK_POINTER(penStartupSrc, APP_EINVAL);
    uint32_t u8PowerSource = 0;
    void *vir_addr = NULL;
    int32_t dev_fd = -1;

    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY);
    if (dev_fd < 0) {
        CVI_LOGE("open /dev/mem/ failed !\n");
        return -1;
    }
    vir_addr = (void *)mmap(NULL, 0xfc, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, POWER_WAKEUP_SOURCE);
    if (vir_addr != NULL) {
        u8PowerSource = *(uint32_t *)(vir_addr + 0xf8);
        CVI_LOGD("u8PowerSource == 0X%x \n", u8PowerSource);
        munmap((void*)vir_addr, 0xfc);
        close(dev_fd);
	}
    //bit19 = 0 is PWR_BUTTON1 wakeup
    //bit17 = 1 is PWR_ON wakeup
    //bit21 = 1 is PWR_WAKEUP0 wakeup
    //bit22 = 1 is PWR_WAKEUP1 wakeup
    if ((u8PowerSource>>19 & 0x01) == 0) {
        *penStartupSrc = SYSTEM_STARTUP_SRC_STARTUP;
    } else {
        if ((u8PowerSource>>17 & 0x01) == 1) {
            CVI_LOGD("%s: gsensorvalue === %d", __func__, (u8PowerSource>>21 & 0x01));
            *penStartupSrc = SYSTEM_STARTUP_SRC_GSENSORWAKEUP;
        } else if((u8PowerSource>>21 & 0x01) == 1) {
            *penStartupSrc = SYSTEM_STARTUP_SRC_USBWAKEUP;
        } else {
            *penStartupSrc = SYSTEM_STARTUP_SRC_USBWAKEUP;
        }
    }

    return 0;
}

int32_t SYSTEM_BootSound(MAPI_AO_HANDLE_T AoHdl)
{
    int32_t s32Ret = 0;
    AUDIO_FRAME_S stFrame = {0};
    uint32_t u32DataLen = (sizeof(g_bootsound)/sizeof(g_bootsound[0]));
    uint32_t u32FrameCnt = 0, s32FrameBytes = 0;
    MAPI_AO_CTX_T *et = (MAPI_AO_CTX_T *)AoHdl;
    unsigned char *pBuffer = g_bootsound;

    s32Ret = MAPI_AO_Start(AoHdl, et->AoChn);
    if (s32Ret != 0) {
        CVI_LOGE("MAPI_AO_Start failed !\n");
    }
    s32FrameBytes = et->attr.u32PtNumPerFrm * et->attr.AudioChannel * 2;

    MAPI_AO_SetAmplifier(AoHdl, true);

    while (u32FrameCnt < (u32DataLen / s32FrameBytes)) {
        stFrame.u64VirAddr[0] = pBuffer;
        stFrame.u32Len = et->attr.u32PtNumPerFrm;
        stFrame.u64TimeStamp = 0;
        stFrame.enSoundmode = (et->attr.AudioChannel==2)?1:0;
        stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;

        s32Ret = MAPI_AO_SendFrame(AoHdl, et->AoChn, &stFrame, 1000);
        if(s32Ret) {
            CVI_LOGE("MAPI_AO_SendFrame failed!\n");
            continue;
        }
        pBuffer += s32FrameBytes;
        u32FrameCnt++;
    }

    //delay close power amplifier
    // Totalsize/(samples*(depth/8)*channels) = duration(s), s - (Totalsize/Data under each time) = sleep time(ms)
    double transfer = (et->attr.enSampleRate * ((et->attr.enSampleRate / 1000) / 8));
    double AudioTotalduration = (u32DataLen / transfer);
    double Aosleeptime = (AudioTotalduration * 1000);
    usleep(Aosleeptime * 1000);
    MAPI_AO_SetAmplifier(AoHdl, false);

    MAPI_AO_Stop(AoHdl, et->AoChn);
    return 0;
}
