#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include "mapi.h"
#include "cvi_math.h"
#include "osal.h"
#include "cvi_log.h"

#include "mapi_adec.h"
#include "mapi_ao.h"


#define STRING_NAME_LEN (128)
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


char codec_name[][STRING_NAME_LEN]={"aac","g711a","g711u"};

#if 1
void dumprawdata(const char *filename,const char *buffer, unsigned int len)
{
    if(!filename) {
        return;
    }
    FILE *fp = fopen(filename, "ab");
    if(!fp) {
        return;
    }
    fwrite(buffer, 1 ,len, fp);
    fclose(fp);
}
#endif


static void print_usage(char * const *argv)
{
    printf("Usage: %s -i wait_decode -o raw_out -C g711u -c 2 -r 8000\n", argv[0]);
    printf("-C :g711u g711a aac \n");
    printf("-c : 2 or 1 (channel count)\n");
    printf("-r : 8000 16000 48000 (sample rate)\n");
}

int decoding = 1;

void sigint_handler(int sig )
{
    decoding = 0;
}

MAPI_AUDIO_CODEC_E GetCodecIndex(const char *codecname)
{
    int ret,find;
    unsigned i;
    for(i = 0; i < (sizeof(codec_name)/STRING_NAME_LEN);i++) {
        ret = strcmp(codecname,codec_name[i]);
        if(ret == 0) {
            find = 1;
            break;
        }
    }

    if(find) {
        return (MAPI_AUDIO_CODEC_E)i;
    }
    return MAPI_VCODEC_MAX;
}


int main(int argc, char *argv[])
{
    int ret;
    FILE *audio_file = NULL;
    FILE *out_file = NULL;
    int channels=0,sample_rate=0;
    char *codecname = NULL;
    char *src_filename = NULL;
    char *out_rawfilename = NULL;
    char out_filename[STRING_NAME_LEN]={0};
    struct wav_header header;
    MAPI_ADEC_HANDLE_T AdecHdl;
    MAPI_ADEC_ATTR_S stAdecAttr;
    AUDIO_STREAM_S stAudioStream;
    MAPI_AUDIO_FRAME_INFO_S stAudFrmInfo = {0};
    uint32_t total_readbytes= 0;
    uint32_t decode_frame = 320;
    uint32_t u32ReadLen;
    int c;
    unsigned char *pBuffer = NULL;
    MAPI_AO_ATTR_S stAoAttr;
    MAPI_AO_HANDLE_T AoHdl;

    // CVI_LOG_INIT();


    if(argc < 2 ) {
        print_usage(argv);
        return 0;
    }

    while ((c = getopt(argc, argv, "i:c:C:r:o:")) != -1) {
        switch (c) {
        case 'C':
            codecname = optarg;
            break;
        case 'i':
            src_filename = optarg;
            break;
        case 'o':
            out_rawfilename = optarg;
            break;
        case 'r':
            sample_rate = atoi(optarg);
            break;
        case 'c':
            channels = atoi(optarg);
            break;
        default:
            print_usage(argv);
            break;

        }
    }

    if (!codecname||!src_filename|| !channels || !sample_rate) {
        CVI_LOGE("invalid input params\n");
        print_usage(argv);
        return 1;
    }
    memset(&stAdecAttr,0,sizeof(MAPI_ADEC_ATTR_S));
    stAoAttr.enSampleRate=sample_rate;
    stAoAttr.channels= channels;
    stAoAttr.u32PtNumPerFrm=320;
    ret = MAPI_AO_Init(&AoHdl,&stAoAttr);
    if(ret){
        CVI_LOGE("MAPI_AO_Init() failed\n");
        return ret;
    }

    snprintf(out_filename,STRING_NAME_LEN,"%s.wav",src_filename);

    audio_file = fopen(src_filename,"rb");
    if (!audio_file) {
        CVI_LOGE("Unable to open file '%s'\n", src_filename);
        return 1;
    }

    out_file = fopen(out_filename, "wb");
    if (!out_file) {
        CVI_LOGE("Unable to create file '%s'\n", out_filename);
        return 1;
    }

    header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = 1;
    header.num_channels = channels;
    header.sample_rate = sample_rate;
    header.bits_per_sample = 16;
    header.byte_rate = (header.bits_per_sample / 8) * channels * sample_rate;
    header.block_align = channels * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;
    fseek(out_file, sizeof(struct wav_header), SEEK_SET);

    stAdecAttr.enAdecFormat = GetCodecIndex(codecname);
    if(stAdecAttr.enAdecFormat >= MAPI_AUDIO_CODEC_BUTT) {
        CVI_LOGE("do not support %s.\n",codecname);
        print_usage(argv);
        return -1;
    }

    if(strcmp(codecname,"aac") == 0)
        decode_frame = 1024;

    stAdecAttr.enMode = ADEC_MODE_STREAM;
    stAdecAttr.enSamplerate = sample_rate;
    stAdecAttr.enSoundmode =channels -1;
    stAdecAttr.frame_size = decode_frame;

    signal(SIGINT, sigint_handler);
    signal(SIGHUP, sigint_handler);
    signal(SIGTERM, sigint_handler);

    pBuffer = (unsigned char *)malloc(CVI_MAX_AUDIO_STREAM_LEN);
    if (pBuffer == NULL) {
        CVI_LOGE("malloc failed.\n");
        return -1;
    }

    MAPI_ADEC_Init(&AdecHdl,&stAdecAttr);
    while(decoding) {
        stAudioStream.pStream = pBuffer;
        u32ReadLen = fread(stAudioStream.pStream, 1, decode_frame*2, audio_file);
        if (u32ReadLen ==  0) {
            ret = MAPI_ADEC_SendEndOfStream(AdecHdl);
            break;
        }

        stAudioStream.u32Len = u32ReadLen;
        ret = MAPI_ADEC_SendStream(AdecHdl, &stAudioStream, CVI_TRUE);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("MAPI_ADEC_SendStream send failed,ret:%d.\n",ret);
            break;
        }

        ret = MAPI_ADEC_GetFrame(AdecHdl, &stAudFrmInfo, CVI_FALSE);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("MAPI_ADEC_SendStream send failed,ret:%d.\n",ret);
            break;
        }

        if(stAudFrmInfo.u32Len) {
            uint32_t write_len_bytes = stAudFrmInfo.u32Len * channels * 2;

            if(out_rawfilename) {
                dumprawdata(out_rawfilename,(char *)stAudFrmInfo.pstream,write_len_bytes);
            }
                fwrite((char *)stAudFrmInfo.pstream,1,write_len_bytes,out_file);
            total_readbytes += write_len_bytes;
        }else {
            printf("finally adec\n");
            break;
        }
        MAPI_ADEC_ReleaseFrame(AdecHdl,&stAudFrmInfo);
    }

    CVI_LOGI("total_bytes:%d\n",total_readbytes);
    header.data_sz = total_readbytes/(2*channels) * header.block_align;
    header.riff_sz = header.data_sz + sizeof(header) - 8;
    fseek(out_file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct wav_header), 1, out_file);

    fclose(audio_file);
    TEST_PASS;
    return 0;
}
