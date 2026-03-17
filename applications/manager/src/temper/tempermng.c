#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/prctl.h>
#include "cvi_log.h"
#include "cvi_sys.h"
#include "tempermng.h"

/*the cpu div is between 1000M and 250M*/
enum MAX_CPU_HZ
{
    HZ_1000M_CPU = 1,
    HZ_500M_CPU,
    HZ_333M_CPU,
    HZ_250M_CPU,
    HZ_BUTT_CPU
};

/*the tpu is between 750M and 187.5M*/
enum MAX_TPU_HZ
{
    HZ_750M_TPU = 1,
    HZ_375M_TPU,
    HZ_250M_TPU,
    HZ_187M_TPU,
    HZ_BUTT_TPU
};

#define CPU_TPU_CLK_ADD 0x03002030
#define CPU_DIV_ADD 0x03002040
#define TPU_DIV_ADD 0x03002054
int32_t cpu_tpu_contrl(int32_t cpu_div, int32_t tpu_div)
{
    void *vir_addr = NULL;
    vir_addr = CVI_SYS_Mmap(CPU_TPU_CLK_ADD, 0x4);
    if (vir_addr != NULL)
    {
		*(uint32_t *)vir_addr |= (0x1<<0);
        *(uint32_t *)vir_addr |= (0x1<<3);
		CVI_SYS_Munmap(vir_addr, 0x4);
	}

    vir_addr = CVI_SYS_Mmap(CPU_DIV_ADD, 0x4);
    if (vir_addr != NULL)
    {
        *(uint32_t *)vir_addr &= ~(0x1F<<16);
		*(uint32_t *)vir_addr |= (cpu_div<<16);
		CVI_SYS_Munmap(vir_addr, 0x4);
	}

    vir_addr = CVI_SYS_Mmap(TPU_DIV_ADD, 0x4);
    if (vir_addr != NULL)
    {
        *(uint32_t *)vir_addr &= ~(0x1F<<16);
		*(uint32_t *)vir_addr |= (tpu_div<<16);
		CVI_SYS_Munmap(vir_addr, 0x4);
	}

    vir_addr = CVI_SYS_Mmap(CPU_TPU_CLK_ADD, 0x4);
    if (vir_addr != NULL)
    {
		*(uint32_t *)vir_addr &= ~(0x1<<0);
        *(uint32_t *)vir_addr &= ~(0x1<<3);
		CVI_SYS_Munmap(vir_addr, 0x4);
	}
    return 0;
}

int32_t get_tempetor(int32_t *temp_value)
{
    char val[32] = {0};
    FILE* fp = NULL;
    char *buf = "/sys/class/thermal/thermal_zone0/temp";

    if(NULL == temp_value)
    {
        CVI_LOGI("point is null\n");
        return -1;
    }

    fp = fopen(buf, "r");
    if(fp != NULL)
    {
        if(!feof(fp))
        {
            fgets(val, sizeof(val), fp);
            if(val != NULL)
            {
                *temp_value = strtoul(val, NULL, 0) /1000;
            }
        }
    }
    else
    {
        CVI_LOGE("get_tempetor failed!\n");
        return -1;
    }
    fclose(fp);
    return 0;
}

void* deal_higt_temperature(void* ctx)
{
    (void)ctx;
    pthread_detach(pthread_self());
    int32_t temperature = 0;
    static int32_t status = HZ_1000M_CPU;
    while(1)
    {
        sleep(1);
        get_tempetor(&temperature);
        if(temperature >= 100 && status != HZ_500M_CPU)
        {
            /*500M CPU, 375M TPU*/
            cpu_tpu_contrl(HZ_500M_CPU, HZ_375M_TPU);
            status = HZ_500M_CPU;
            CVI_LOGE("cpu 500M HZ now, temperature = %d\n", temperature);
        }
        else if(temperature < 100 && status!= HZ_1000M_CPU)
        {
            /*1G CPU, 750M TPU*/
            cpu_tpu_contrl(HZ_1000M_CPU, HZ_750M_TPU);
            status = HZ_1000M_CPU;
            CVI_LOGE("cpu 1000M HZ now, temperature = %d\n", temperature);
        }
    }
    pthread_exit(0);
}


int32_t Detect_Temperature_Thread(void)
{
    int32_t s32Ret = 0;
    pthread_t taskid;

    s32Ret = pthread_create(&taskid, NULL, deal_higt_temperature, NULL);
    if (s32Ret != 0)
    {
        CVI_LOGE("CVI_pthread_create failed\n");
        return -1;
    }
    return 0;
}