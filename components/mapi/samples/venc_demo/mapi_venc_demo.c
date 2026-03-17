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
#include <fcntl.h>
#include <pthread.h>
#include "mapi.h"
#include "osal.h"
#include "cvi_log.h"

#define CODEC_NAME_STR_LEN 10
#define ALIGNED_WIDTH       32
#define ALIGNED_MEM         0x1000
#define MAX_GETOPT_OPTIONS  64
#define MAX_STRING_LEN      255
#define MAX_FILENAME_LEN    64
#define MULTI_ENCODE_MAX    4

typedef struct _INPUT_CONFIG_T{
    uint32_t image_width;
    uint32_t image_height;
    MAPI_VCODEC_E eVCodec;
    char *inputfile;
    char *outputfile;
    MAPI_VENC_HANDLE_T venc;
    MAPI_VENC_CHN_ATTR_T venc_attr;
    VIDEO_FRAME_INFO_S *pstFrameInfo;
}INPUT_CONFIG_T;



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

char codec_name[][CODEC_NAME_STR_LEN]={"h264","h265","jpg","mjpg"};
static pthread_t pVencTask[VENC_MAX_CHN_NUM];
//static pthread_mutex_t getstream_mutex = PTHREAD_MUTEX_INITIALIZER;



static void print_usage(char * const *argv)
{
    printf("// ------------------------------------------------\n");
    printf("%s  -c h264 -w 1920 -h 1080 -i inputfile1 inputfile2 .. \n", argv[0]);
    printf("support codec:");
    for(uint32_t i= 0; i < (sizeof(codec_name)/CODEC_NAME_STR_LEN); i++) {
        printf(" %s",codec_name[i]);
    }
    printf("\n");
    printf("// ------------------------------------------------\n");
}


static uint32_t getFrameSize(uint32_t w, uint32_t h, PIXEL_FORMAT_E fmt)
{
    if (fmt == PIXEL_FORMAT_YUV_PLANAR_420) {
        return (w * h * 3) >> 1;
    } else if (fmt == PIXEL_FORMAT_YUV_PLANAR_422) {
        return (w * h * 2);
    } else {
        CVI_LOG_ASSERT(0, "Unsupported fmt %d\n", fmt);
    }
    return 0;
}

MAPI_VCODEC_E GetCodecIndex(const char *codecname)
{
    int ret,find;
    unsigned i;
    for(i = 0; i < (sizeof(codec_name)/CODEC_NAME_STR_LEN);i++) {
        ret = strcmp(codecname,codec_name[i]);
        if(ret == 0) {
            find = 1;
            break;
        }
    }

    if(find) {
        return (MAPI_VCODEC_E)i;
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
    fwrite(buffer,len, 1,fp);
    fclose(fp);
}

static int VencStreamProcThreadLoop(void *pArgs)
{
    CVI_U64 t1, t2;
    INPUT_CONFIG_T *pstInputConfig = (INPUT_CONFIG_T *)pArgs;

    char *inputfile = pstInputConfig->inputfile;
    char *outputfile = pstInputConfig->outputfile;

    FILE *fpSrc = fopen(inputfile, "rb");
    if (fpSrc == NULL) {
        CVI_LOGE("Input file %s open failed !\n", inputfile);
        return CVI_FAILURE;
    }

    uint32_t frame_size = getFrameSize(pstInputConfig->image_width, pstInputConfig->image_height, pstInputConfig->venc_attr.venc_param.pixel_format);
    char *yuv_data = (char *)malloc(frame_size);
    if (!yuv_data) {
        CVI_LOGE("malloc failed\n");
        return CVI_FAILURE;
    }

    fseek(fpSrc, 0, SEEK_END);
    unsigned int file_size = ftell(fpSrc);
    unsigned int num_frames = file_size / frame_size;
    CVI_LOGI("file_size %u\n", file_size);
    CVI_LOGI("input:%s u32FrameSize %u,num_frames %u\n", inputfile,frame_size,num_frames);

    rewind(fpSrc);

    for (uint32_t i = 0; i < num_frames; i++) {
        VIDEO_FRAME_INFO_S frame;
        fread((void *)yuv_data, 1, frame_size, fpSrc);
        CHECK_RET(MAPI_GetFrameFromMemory_YUV(&frame,
                pstInputConfig->venc_attr.venc_param.width, pstInputConfig->venc_attr.venc_param.height,
                pstInputConfig->venc_attr.venc_param.pixel_format, yuv_data));

        VENC_STREAM_S stream = {0};
        OSAL_TIME_GetBootTimeUs(&t1);
        CHECK_RET(MAPI_VENC_EncodeFrame(pstInputConfig->venc,&frame,&stream));
        OSAL_TIME_GetBootTimeUs(&t2);

        for (uint32_t j = 0; j < stream.u32PackCount; j++) {
            VENC_PACK_S *ppack;
            ppack = &stream.pstPack[j];
            dumpdata(outputfile,(char *)(ppack->pu8Addr + ppack->u32Offset),ppack->u32Len - ppack->u32Offset);
        }
        CHECK_RET(MAPI_ReleaseFrame(&frame));
        CVI_LOGI("      Venc    %2.2f ms\n", (t2-t1)/1000.0);
        CHECK_RET(MAPI_VENC_ReleaseStream(pstInputConfig->venc, &stream));
    }

    CHECK_RET(MAPI_VENC_StopRecvFrame(pstInputConfig->venc));
    CHECK_RET(MAPI_VENC_DeinitChn(pstInputConfig->venc));
    fclose(fpSrc);
    free(yuv_data);

    return CVI_SUCCESS;
}


static void *VencStreamProcThread(void *pArgs)
{
    VencStreamProcThreadLoop(pArgs);
    pthread_exit(0);
    return NULL;
}


int main(int argc, char *argv[])
{
    char outputfile[MULTI_ENCODE_MAX][MAX_FILENAME_LEN]={0};
    char *inputfile[MULTI_ENCODE_MAX];
    char *codecname = NULL;
    CVI_S32 s32Ret = CVI_SUCCESS;
    MAPI_VCODEC_E eVCodeIndex;
    int height,width;
    INPUT_CONFIG_T *astInputConfig[MULTI_ENCODE_MAX];
    uint32_t ChannelCount = 0;
    int c;

    opterr = 0;
    height = width = 0;
    // CVI_LOG_INIT();
    while ((c = getopt(argc, argv, "c:i:h:w:")) != -1) {
        switch (c) {
        case 'i':
            inputfile[0] = optarg;
            ChannelCount = 1;
            printf("-i optind:%d.\n",optind);
            int k =0;
            while((k < MULTI_ENCODE_MAX-1) && argv[optind +k])
            {
                inputfile[k+1] = argv[optind+k];
                printf("optind=%d,argv[%d]:%s\n",k,k,inputfile[k+1]);
                k++;
            }
            ChannelCount += k;
            printf("final input file count k = %d.\n",ChannelCount);
            break;
        case 'c':
            codecname = optarg;
        case 'h':
            height = atoi(optarg);
            break;
        case 'w':
            width = atoi(optarg);
            break;
        default:
            printf("Invalid option : %c\n", c);
            print_usage(argv);
            abort ();
        }
    }

    if(!ChannelCount || !codecname || !height || !width) {
        CVI_LOGE("error,NULL params.\n");
        print_usage(argv);
        return CVI_FAILURE;
    }

    eVCodeIndex = GetCodecIndex(codecname);
    if(MAPI_VCODEC_MAX == eVCodeIndex) {
        CVI_LOGE("do not support %s.\n",codecname);
        print_usage(argv);
        return CVI_FAILURE;
    }
    if(MAPI_VCODEC_JPG == eVCodeIndex && ChannelCount> 1) {
        ChannelCount = 1;
        CVI_LOGW("jpg can not support multi channel,only convert first pic.\n");
    }

    memset(outputfile,0,sizeof(outputfile));

    for(uint32_t i= 0; i < ChannelCount; i++) {
        strcat(outputfile[i],inputfile[i]);
        strcat(outputfile[i],".");
        strcat(outputfile[i],codecname);
        astInputConfig[i] = (INPUT_CONFIG_T *)malloc(sizeof(INPUT_CONFIG_T));
        memset(astInputConfig[i],0,sizeof(INPUT_CONFIG_T));
        astInputConfig[i]->inputfile = inputfile[i];
        astInputConfig[i]->outputfile = outputfile[i];
        astInputConfig[i]->image_width = width;
        astInputConfig[i]->image_height = height;
        astInputConfig[i]->eVCodec = eVCodeIndex;
        astInputConfig[i]->venc_attr.venc_param.pixel_format = PIXEL_FORMAT_YUV_PLANAR_420;
        astInputConfig[i]->venc_attr.venc_param.codec = eVCodeIndex;
        astInputConfig[i]->venc_attr.venc_param.width = width;
        astInputConfig[i]->venc_attr.venc_param.height = height;
        astInputConfig[i]->venc_attr.venc_param.rate_ctrl_mode = 4;
        astInputConfig[i]->venc_attr.venc_param.bitrate_kbps = 8000;
        astInputConfig[i]->venc_attr.venc_param.iqp = 46;
        astInputConfig[i]->venc_attr.venc_param.pqp = 46;
        astInputConfig[i]->venc_attr.venc_param.maxIqp = 60;
        astInputConfig[i]->venc_attr.venc_param.minIqp = 30;
        astInputConfig[i]->venc_attr.venc_param.maxQp = 60;
        astInputConfig[i]->venc_attr.venc_param.minQp = 30;
        CVI_LOGI("outputfile[%d]:%s eVCodec:%d\n",i,outputfile[i],eVCodeIndex);
    }

    //sys init
    MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = width;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = height;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[0].vb_blk_num = 10;
    sys_attr.vb_pool_num = 1;
    CHECK_RET(MAPI_Media_Init(&sys_attr));

    //venc init
    for(uint32_t idx = 0; idx < ChannelCount; idx++) {
        s32Ret = MAPI_VENC_InitChn(&astInputConfig[idx]->venc, &astInputConfig[idx]->venc_attr);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("MAPI_VENC_InitChn, %d\n", s32Ret);
            return CVI_FAILURE;
        }
        s32Ret = MAPI_VENC_StartRecvFrame(astInputConfig[idx]->venc, -1);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("MAPI_VENC_StartRecvFrame, %d\n", s32Ret);
            return CVI_FAILURE;
        }
    }
    CVI_LOGI("venc init suc.\n");

    for(uint32_t idx = 0; idx < ChannelCount; idx++) {
        pthread_create(&pVencTask[idx],0,VencStreamProcThread,(void *)astInputConfig[idx]);
        pthread_detach(pVencTask[idx]);
    }

    getchar();

    CHECK_RET(MAPI_Media_Deinit());


    return 0;
}
