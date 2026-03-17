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
#include "cvi_log.h"

#include "mapi_aenc.h"
#include "mapi_acap.h"
//#include "cvi_audio_aac.h"

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

struct riff_wave_header {
    unsigned int riff_id;
    unsigned int riff_sz;
    unsigned int wave_id;
};

struct chunk_header {
    unsigned int id;
    unsigned int sz;
};

struct chunk_fmt {
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
};


typedef struct CVI_AENC_CB_INFO_T{
    MAPI_AUDIO_CODEC_E enCodecType;
    uint32_t u32PtNumPerFrm;
    char * out_filename;
}CVI_AENC_CB_INFO;

char codec_name[][STRING_NAME_LEN]={"aac","g711a","g711u"};

static void print_usage(char * const *argv)
{
    printf("Usage: %s -i wait_enc.wav -o outfile -C g711u\n", argv[0]);
    printf("support codec:g711u g711a aac\n");
}

int encoding = 1;

void sigint_handler(int sig )
{
    encoding = 0;
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



void dumpdata(const char *filename,const char *buffer, unsigned int len)
{
    if(!filename) {
        return;
    }
    FILE *fp = fopen(filename, "ab");
    if(!fp) {
        return;
    }
    fwrite(buffer,1, len,fp);
    fclose(fp);
}

FILE *open_wav_file(const char *filename,int *channel,int *sample_rate)
{
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
    int more_chunks = 1;
    FILE *file = NULL;
    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        return NULL;
    }

    fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
            fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
            fclose(file);
            return NULL;
    }


    do {
        fread(&chunk_header, sizeof(chunk_header), 1, file);

        switch (chunk_header.id) {
        case ID_FMT:
            fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
            *channel = chunk_fmt.num_channels;
            *sample_rate = chunk_fmt.sample_rate;
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(chunk_fmt))
                fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = 0;
            chunk_header.sz = le32toh(chunk_header.sz);
            break;
        default:
            /* Unknown chunk, skip bytes */
            fseek(file, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    TEST_PASS;
    return file;
}


int  Aenc_GetEncodeDataCB(MAPI_AENC_HANDLE_T AencHdl,AUDIO_STREAM_S* pAStreamData, void *pPrivateData)
{
    CVI_AENC_CB_INFO *pstAencCbInfo = (CVI_AENC_CB_INFO *)pPrivateData;
    CVI_LOGI("codec_type:%d.\n",pstAencCbInfo->enCodecType);

    if(pstAencCbInfo && pstAencCbInfo->out_filename) {
        dumpdata(pstAencCbInfo->out_filename, (char *)pAStreamData->pStream, pAStreamData->u32Len);
    }

    return MAPI_SUCCESS;
}


int main(int argc, char *argv[])
{
    int ret;
    FILE *audio_file = NULL;
    FILE *audio_out = NULL;
    int channels,sample_rate;
    char *codecname = NULL;
    char *filename = NULL;
    char *outfilename = NULL;
    MAPI_AENC_HANDLE_T AencHdl;
    MAPI_AENC_ATTR_S stAencAttr;
    AUDIO_FRAME_S stFrame;
    uint32_t encode_length;
    int c;
    struct timespec end;
    struct timespec now;
    uint32_t cap_time = 20;
    MAPI_ACAP_HANDLE_T AcapHdl;
    MAPI_ACAP_ATTR_S stACapAttr;

    AEC_FRAME_S stAecFrm;
    AUDIO_STREAM_S stStream;
    // CVI_LOG_INIT();

    if(argc < 2 ) {
        print_usage(argv);
        return 0;
    }

    while ((c = getopt(argc, argv, "i:C:o:")) != -1) {
        switch (c) {
        case 'C':
            codecname = optarg;
            break;
        case 'i':
            filename = optarg;
            break;
        case 'o':
            outfilename = optarg;
            break;
        default:
            print_usage(argv);
            //return 1;
            break;

        }
    }

    if (!codecname||!outfilename) {
        CVI_LOGE("invalid input params\n");
        print_usage(argv);
        return 1;
    }

    if(filename) {
        audio_file = open_wav_file(filename,&channels,&sample_rate);
    }else {
        channels = 2;
        sample_rate = 8000;
    }

    signal(SIGINT, sigint_handler);
    signal(SIGHUP, sigint_handler);
    signal(SIGTERM, sigint_handler);

    audio_out = fopen(outfilename,"ab");
    if (audio_out == NULL) {
        CVI_LOGE("out file open fail\n");
        return -1;
    }

    //20*channels * 2 * sample_rate /1000; //10ms data
    encode_length = 320;
    char *pBuff = (char *)malloc(encode_length*2*channels);
    stAencAttr.enAencFormat = GetCodecIndex(codecname);
    stAencAttr.src_samplerate = sample_rate;
    if(stAencAttr.enAencFormat >= MAPI_AUDIO_CODEC_BUTT) {
        CVI_LOGE("do not support %s.\n",codecname);
        print_usage(argv);
        return -1;
    }
    stAencAttr.u32PtNumPerFrm  =encode_length;
    stAencAttr.channels = channels;

    stACapAttr.channel = channels;
    stACapAttr.enSampleRate = (AUDIO_SAMPLE_RATE_E)sample_rate;
    stACapAttr.u32PtNumPerFrm = encode_length;

    stACapAttr.bVqeOn = 1;

    MAPI_ACAP_Init(&AcapHdl,&stACapAttr);

    MAPI_ACAP_Start(AcapHdl);
    MAPI_AENC_Init(&AencHdl,&stAencAttr);
    MAPI_AENC_Start(AencHdl);


    if(!audio_file) {
        MAPI_AENC_BindACap(0,0,0,0);
    }

    stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
    stFrame.enSoundmode = (channels==2)?AUDIO_SOUND_MODE_STEREO:AUDIO_SOUND_MODE_MONO;
    stFrame.u64VirAddr[0] = (unsigned char *)malloc(encode_length*2*channels);

    while(encoding && audio_file) { //not bind acap flow
        memset(pBuff,0,encode_length*2*channels);
        ret = fread(pBuff,2*channels,encode_length,audio_file);
        if(ret == 0) break;
        memcpy( stFrame.u64VirAddr[0],pBuff,encode_length*channels*2);//320*2*2
        stFrame.u32Len = encode_length;
        MAPI_AENC_SendFrame(AencHdl,&stFrame,&stAecFrm);
        MAPI_AENC_GetStream(AencHdl,&stStream,0);

        fwrite(stStream.pStream,1,stStream.u32Len,audio_out);
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    end.tv_sec = now.tv_sec + cap_time;
    end.tv_nsec = now.tv_nsec;


    while(encoding && !audio_file) {

        if (cap_time) {
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (now.tv_sec > end.tv_sec ||
                (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec))
                break;
        }


		ret = MAPI_AENC_GetStream(AencHdl,&stStream,0);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("[error],[%s],[line:%d],\n", __func__, __LINE__);
			break;
		}

		if(!stStream.u32Len) {
			continue;
		}
        fwrite(stStream.pStream,1,stStream.u32Len,audio_out);
}


    if(!audio_file) {
        MAPI_AENC_UnbindACap(0,0,0,0);
    }

    if(audio_file) {
        fclose(audio_file);
    }
    fclose(audio_out);
    MAPI_AENC_Stop(AencHdl);
    MAPI_ACAP_Stop(AcapHdl);
    MAPI_AENC_Deinit(AencHdl);
    MAPI_ACAP_Deinit(AcapHdl);
    TEST_PASS;
    return 0;
}
