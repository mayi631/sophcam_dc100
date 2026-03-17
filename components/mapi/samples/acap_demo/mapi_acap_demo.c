#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <pthread.h>
#include "mapi.h"
#include "cvi_math.h"
#include "cvi_log.h"

#include "mapi_acap.h"

#define FILE_NAME_LEN (128)
#ifndef CHECK_RET
#define CHECK_RET(express)                                                    \
    do {                                                                      \
        int rc = express;                                                     \
        if (rc != 0) {                                                        \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n",                       \
                    __FILE__, __LINE__, rc);                                  \
            return rc;                                                        \
        }                                                                     \
    } while (0)
#endif

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define TEST_PASS printf("------\nSUCCESS\n------\n")

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};


static void print_usage(char * const *argv)
{
    printf("Usage: %s file.wav "
                " [-c channels] [-r rate] [-p period_size]"
                " [-T capture time]\n", argv[0]);
}

int capturing = 1;

void sigint_handler(int sig )
{
    capturing = 0;
}


void dumpdata(const char *filename,const char *buffer, unsigned int len)
{
    if(!filename) {
        return;
    }
    FILE *fp = fopen(filename, "ab");
    if(!fp) {
        return;
    }
    fwrite(buffer,len, 1,fp);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    uint32_t ChannelCount = 2;
    uint32_t samplerate = 16000;
    uint32_t outSampleRate = 16000;
    uint32_t u32PtNumPerFrm = 320;
    uint32_t cap_time = 0;
    int ret = CVI_SUCCESS;
    FILE *file;
    struct wav_header header;
    uint32_t readbytes = 0;
    uint32_t total_readbytes= 0;
    struct timespec end;
    struct timespec now;
    MAPI_ACAP_HANDLE_T AcapHdl;
    MAPI_ACAP_ATTR_S stACapAttr;
    AUDIO_FRAME_S stFrame;
    AEC_FRAME_S   stAecFrm;


    // CVI_LOG_INIT();
    if (argc < 2) {
        print_usage(argv);
        return 1;
    }
    file = fopen(argv[1], "wb");
    if (!file) {
        CVI_LOGE("Unable to create file '%s'\n", argv[1]);
        return 1;
    }

    argv += 2;
    while (*argv) {
        if (strcmp(*argv, "-c") == 0) {
            argv++;
            if (*argv)
                ChannelCount = atoi(*argv);
        } else if (strcmp(*argv, "-r") == 0) {
            argv++;
            if (*argv)
                samplerate = atoi(*argv);
        } else if (strcmp(*argv, "-p") == 0) {
            argv++;
            if (*argv)
                u32PtNumPerFrm = atoi(*argv);
        } else if (strcmp(*argv, "-T") == 0) {
            argv++;
            if (*argv)
                cap_time = atoi(*argv);
        } else if (strcmp(*argv, "-R") == 0) {
            argv++;
            if (*argv)
                outSampleRate = atoi(*argv);
        }
        if (*argv)
            argv++;
    }

    signal(SIGINT, sigint_handler);
    signal(SIGHUP, sigint_handler);
    signal(SIGTERM, sigint_handler);

    int OutputChannel = ChannelCount;
    //char *pBuff = (char *)malloc(u32PtNumPerFrm*ChannelCount*2);
    stACapAttr.channel = ChannelCount;
    stACapAttr.enSampleRate = (AUDIO_SAMPLE_RATE_E)samplerate;
    stACapAttr.u32PtNumPerFrm = u32PtNumPerFrm;
    stACapAttr.bVqeOn = 0;
    header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = 1;
    if (stACapAttr.bVqeOn)
        OutputChannel = 2;
    header.num_channels = OutputChannel;
    if (outSampleRate != stACapAttr.enSampleRate)
        header.sample_rate = outSampleRate;
    else
        header.sample_rate = samplerate;
    header.bits_per_sample = 16;
    header.byte_rate = (header.bits_per_sample / 8) * OutputChannel * samplerate;
    header.block_align = OutputChannel * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;
    fseek(file, sizeof(struct wav_header), SEEK_SET);

    MAPI_ACAP_Init(&AcapHdl,&stACapAttr);
    MAPI_ACAP_Start(AcapHdl);

    if (outSampleRate != stACapAttr.enSampleRate)
        MAPI_ACAP_EnableReSmp(AcapHdl, (AUDIO_SAMPLE_RATE_E)outSampleRate);


    clock_gettime(CLOCK_MONOTONIC, &now);
    end.tv_sec = now.tv_sec + cap_time;
    end.tv_nsec = now.tv_nsec;
    while(capturing)
    {
        // read from ai module
        ret = MAPI_ACAP_GetFrame(AcapHdl, &stFrame, &stAecFrm);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("MAPI_ACAP_GetFrame failed,ret:%d.!!\n",ret);
            usleep(1000*(1000*u32PtNumPerFrm/samplerate)/2);
            continue;
        }
        readbytes = stFrame.u32Len*OutputChannel*2;

        if (fwrite(stFrame.u64VirAddr[0], 1, readbytes, file) != readbytes) {
            CVI_LOGE("Error capturing sample\n");
            ret = CVI_FAILURE;
            break;
        }

        total_readbytes += readbytes;

        ret = MAPI_ACAP_ReleaseFrame(AcapHdl,&stFrame,&stAecFrm);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("MAPI_ACAP_ReleaseFrame, failed with %d!\n",ret);
        }

        if (cap_time) {
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (now.tv_sec > end.tv_sec ||
                (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec))
                break;
        }
    }

    header.data_sz = total_readbytes/(2*OutputChannel) * header.block_align;
    header.riff_sz = header.data_sz + sizeof(header) - 8;
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct wav_header), 1, file);

    fclose(file);

    if (outSampleRate != stACapAttr.enSampleRate)
        MAPI_ACAP_DisableReSmp(AcapHdl);
    MAPI_ACAP_Deinit(AcapHdl);

    if(ret == CVI_SUCCESS)
        TEST_PASS;

    return 0;
}
