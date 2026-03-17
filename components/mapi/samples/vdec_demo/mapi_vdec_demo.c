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
#include <sys/prctl.h>
#include <fcntl.h>
#include <pthread.h>
#include "mapi.h"
#include "cvi_math.h"
#include "osal.h"
#include "cvi_log.h"
#include "cvi_common.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"
//#include "sample_comm.h"
#include "mapi_vdec.h"
#include "cvi_sys.h"
#include "cvi_msg_client.h"

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
    MAPI_VDEC_HANDLE_T venc;
    MAPI_VDEC_CHN_ATTR_T venc_attr;
    VIDEO_FRAME_INFO_S *pstFrameInfo;
}INPUT_CONFIG_T;


typedef struct _MAPI_VDEC_THREAD_PARAM_S {
    MAPI_VDEC_HANDLE_T vdec_hdl;
    MAPI_VCODEC_E eVDecType;
    char cOutFileName[128];
    char cInputFileName[128];
    int s32StreamMode;
    int s32MilliSec;
    int s32MinBufSize;
    int s32IntervalTime;
    CVI_U64 u64PtsInit;
    CVI_U64 u64PtsIncrease;
    bool bCircleSend;
    bool bFileEnd;
} MAPI_VDEC_THREAD_PARAM_S;

#define VCODEC_BUFF_SIZE 2

typedef struct VDEC_PARAMS_T{
        int readpos;
        int writepos;
        pthread_cond_t notempty;
        pthread_cond_t notfull;
        pthread_mutex_t vdec_mutex;
}VDEC_PARAMS;

VDEC_PARAMS gstVdecParams;


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

char codec_name[][CODEC_NAME_STR_LEN]={"h264","h265","jpg"};
PAYLOAD_TYPE_E gacodec_type[]={
    PT_H264,
    PT_H265,
    PT_JPEG,
    PT_BUTT
};



static void print_usage(char * const *argv)
{
    printf("// ------------------------------------------------\n");
    printf("%s -c h264 -i inputfile1 inputfile2 .. \n", argv[0]);
    printf("support codec:");
    for(uint32_t i= 0; i < (sizeof(codec_name)/CODEC_NAME_STR_LEN); i++) {
        printf(" %s",codec_name[i]);
    }
    printf("\n");
    printf("// ------------------------------------------------\n");
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

int dump_yuv(const char *filename, VIDEO_FRAME_S stVFrame)
{
    char *w_ptr;

    CVI_LOGI("write_yuv u32Width = %d, u32Height = %d\n",
            stVFrame.u32Width, stVFrame.u32Height);
    CVI_LOGI("write_yuv u32Stride[0] = %d, u32Stride[1] = %d, u32Stride[2] = %d\n",
            stVFrame.u32Stride[0], stVFrame.u32Stride[1], stVFrame.u32Stride[2]);
    CVI_LOGI("write_yuv u32Length[0] = %d, u32Length[1] = %d, u32Length[2] = %d\n",
            stVFrame.u32Length[0], stVFrame.u32Length[1], stVFrame.u32Length[2]);
    w_ptr = (char *)stVFrame.pu8VirAddr[0];
    for (unsigned int i = 0; i < stVFrame.u32Height; i++) {
        dumpdata(filename,w_ptr + i * stVFrame.u32Stride[0],stVFrame.u32Width);
    }
    w_ptr = (char *)stVFrame.pu8VirAddr[1];
    for (unsigned int i = 0; i < stVFrame.u32Height / 2; i++) {
        dumpdata(filename,w_ptr + i * stVFrame.u32Stride[1],stVFrame.u32Width / 2);
    }
    w_ptr = (char *)stVFrame.pu8VirAddr[2];
    for (unsigned int i = 0; i < stVFrame.u32Height / 2; i++) {
        dumpdata(filename,w_ptr + i * stVFrame.u32Stride[2],stVFrame.u32Width / 2);
    }
    return 0;
}
int send_suc_cnt = 0;
int get_frame_cnt = 0;

void *VDEC_GetPicThread(void *pArgs)
{
    MAPI_VDEC_THREAD_PARAM_S *pstVdecThreadParam = (MAPI_VDEC_THREAD_PARAM_S *)pArgs;
    int s32Ret;
    CVI_U64 t1, t2;
    VIDEO_FRAME_INFO_S stVFrame;

    //prctl(PR_SET_NAME, "VdecGetPic", 0, 0, 0);

    //while (1) {
        OSAL_TIME_GetBootTimeUs(&t1);
        s32Ret = MAPI_VDEC_GetFrame(
                pstVdecThreadParam->vdec_hdl,
                &stVFrame);
        OSAL_TIME_GetBootTimeUs(&t2);
//      CVI_LOGI("  s32Ret:%d    Vdec    %2.2f ms\n",s32Ret, (t2-t1)/1000.0);
        if (s32Ret == CVI_SUCCESS) {
            OSAL_TIME_GetBootTimeUs(&t1);
            dump_yuv(pstVdecThreadParam->cOutFileName, stVFrame.stVFrame);
            OSAL_TIME_GetBootTimeUs(&t2);
//          CVI_LOGI("MAPI_VDEC_GetFrame %d suc,dumpyuv %2.2f ms\n",count++,(t2-t1)/1000.0);
            s32Ret = MAPI_VDEC_ReleaseFrame(pstVdecThreadParam->vdec_hdl, &stVFrame);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("vdec_hdl %p CVI_MPI_VDEC_ReleaseFrame fail for s32Ret=0x%x!\n",
                       pstVdecThreadParam->vdec_hdl, s32Ret);
            }
        }

       usleep(10 * 1000);
    //}

    CVI_VDEC_TRACE("\033[0;35m vdec_hdl %p get pic thread return ...  \033[0;39m\n", pstVdecThreadParam->vdec_hdl);
    //pthread_exit(0);
    return (CVI_VOID *)CVI_SUCCESS;
}

void *VdecSendStreamThread(void *pArgs)
{
    MAPI_VDEC_THREAD_PARAM_S *pstVdecThreadParam = (MAPI_VDEC_THREAD_PARAM_S *)pArgs;
    bool bEndOfStream = false;
    int s32UsedBytes = 0, s32ReadLen = 0;
    FILE *fpStrm = NULL;
    unsigned char *pu8Buf = NULL;
    VDEC_STREAM_S stStream;
    int timeoutcnt = 0;
    bool bFindStart, bFindEnd;
    CVI_U64 u64PTS = 0;
    uint32_t u32Len, u32Start;
    int s32Ret, i;
    pstVdecThreadParam->s32IntervalTime = 10*1000;
    char *cStreamFile = pstVdecThreadParam->cInputFileName;

    prctl(PR_SET_NAME, "VdecSendStreamThread", 0, 0, 0);

    if (cStreamFile != 0) {
        fpStrm = fopen(cStreamFile, "rb");
        CVI_LOGI("cStreamFile = %s\n", cStreamFile);

        if (fpStrm == NULL) {
            CVI_LOGE("vdec_hdl %p can't open file %s in send stream thread!\n",
                    pstVdecThreadParam->vdec_hdl, cStreamFile);
            return (void *)(CVI_FAILURE);
        }
    }

    CVI_LOGI("\n \033[0;36m vdec_hdl %p, stream file:%s, userbufsize: %d \033[0;39m\n",
            pstVdecThreadParam->vdec_hdl,
            pstVdecThreadParam->cInputFileName,
            pstVdecThreadParam->s32MinBufSize);

    pu8Buf = malloc(pstVdecThreadParam->s32MinBufSize);
    if (pu8Buf == NULL) {
        CVI_LOGE("vdec_hdl %p can't alloc %d in send stream thread!\n",
                pstVdecThreadParam->vdec_hdl,
                pstVdecThreadParam->s32MinBufSize);
        fclose(fpStrm);
        return (void *)(CVI_FAILURE);
    }
    fflush(stdout);

    u64PTS = pstVdecThreadParam->u64PtsInit;
    while (1) {
        VDEC_GetPicThread(pArgs);
        bEndOfStream = false;
        bFindStart = false;
        bFindEnd = false;
        u32Start = 0;
        s32Ret = fseek(fpStrm, s32UsedBytes, SEEK_SET);
        s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
        if (s32ReadLen == 0) {
            if (pstVdecThreadParam->bCircleSend == CVI_TRUE) {
                memset(&stStream, 0, sizeof(VDEC_STREAM_S));
                stStream.bEndOfStream = CVI_TRUE;
                bEndOfStream = true;
                MAPI_VDEC_SendStream(pstVdecThreadParam->vdec_hdl,&stStream);
                break;
                //s32UsedBytes = 0;
                //fseek(fpStrm, 0, SEEK_SET);
                //s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
            } else {
                CVI_VDEC_TRACE("break\n");
                break;
            }
        }

        if (pstVdecThreadParam->eVDecType == MAPI_VCODEC_H264) {
            for (i = 0; i < s32ReadLen - 8; i++) {
                int tmp = pu8Buf[i + 3] & 0x1F;

                if (pu8Buf[i] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                    (((tmp == 0x5 || tmp == 0x1) && ((pu8Buf[i + 4] & 0x80) == 0x80)) ||
                     (tmp == 20 && (pu8Buf[i + 7] & 0x80) == 0x80))) {
                    bFindStart = CVI_TRUE;
                    i += 8;
                    break;
                }
            }

            for (; i < s32ReadLen - 8; i++) {
                int tmp = pu8Buf[i + 3] & 0x1F;

                if (pu8Buf[i] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                    (tmp == 15 || tmp == 7 || tmp == 8 || tmp == 6 ||
                     ((tmp == 5 || tmp == 1) && ((pu8Buf[i + 4] & 0x80) == 0x80)) ||
                     (tmp == 20 && (pu8Buf[i + 7] & 0x80) == 0x80))) {
                    bFindEnd = CVI_TRUE;
                    break;
                }
            }

            if (i > 0)
                s32ReadLen = i;
            if (bFindStart == CVI_FALSE) {
                CVI_LOGE("vdec_hdl %p can not find H264 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                       pstVdecThreadParam->vdec_hdl, s32ReadLen, s32UsedBytes);
            }
            if (bFindEnd == CVI_FALSE) {
                s32ReadLen = i + 8;
            }

        } else if (pstVdecThreadParam->eVDecType == MAPI_VCODEC_H265) {
            bool bNewPic = false;

            for (i = 0; i < s32ReadLen - 6; i++) {
                uint32_t tmp = (pu8Buf[i + 3] & 0x7E) >> 1;

                bNewPic = (pu8Buf[i + 0] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                       (tmp <= 21) && ((pu8Buf[i + 5] & 0x80) == 0x80));

                if (bNewPic) {
                    bFindStart = CVI_TRUE;
                    i += 6;
                    break;
                }
            }

            for (; i < s32ReadLen - 6; i++) {
                CVI_U32 tmp = (pu8Buf[i + 3] & 0x7E) >> 1;

                bNewPic = (pu8Buf[i + 0] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                       (tmp == 32 || tmp == 33 || tmp == 34 || tmp == 39 || tmp == 40 ||
                        ((tmp <= 21) && (pu8Buf[i + 5] & 0x80) == 0x80)));

                if (bNewPic) {
                    bFindEnd = CVI_TRUE;
                    break;
                }
            }
            if (i > 0)
                s32ReadLen = i;

            if (bFindStart == CVI_FALSE) {
                CVI_LOGE("vdec_hdl %p can not find H265 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                       pstVdecThreadParam->vdec_hdl, s32ReadLen, s32UsedBytes);
            }
            if (bFindEnd == CVI_FALSE) {
                s32ReadLen = i + 6;
            }

        } else if (pstVdecThreadParam->eVDecType == MAPI_VCODEC_JPG || pstVdecThreadParam->eVDecType == MAPI_VCODEC_MJP) {
            for (i = 0; i < s32ReadLen - 1; i++) {
                if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD8) {
                    u32Start = i;
                    bFindStart = CVI_TRUE;
                    i = i + 2;
                    break;
                }
            }

            for (; i < s32ReadLen - 3; i++) {
                if ((pu8Buf[i] == 0xFF) && (pu8Buf[i + 1] & 0xF0) == 0xE0) {
                    u32Len = (pu8Buf[i + 2] << 8) + pu8Buf[i + 3];
                    i += 1 + u32Len;
                } else {
                    break;
                }
            }

            for (; i < s32ReadLen - 1; i++) {
                if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD9) {
                    bFindEnd = CVI_TRUE;
                    break;
                }
            }
            s32ReadLen = i + 2;

            if (bFindStart == CVI_FALSE) {
                CVI_LOGE("vdec_hdl %p can not find JPEG start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                       pstVdecThreadParam->vdec_hdl, s32ReadLen, s32UsedBytes);
            }
        } else {
            if ((s32ReadLen != 0) && (s32ReadLen < pstVdecThreadParam->s32MinBufSize)) {
                bEndOfStream = CVI_TRUE;
            }
        }

        stStream.u64PTS = u64PTS;
        stStream.pu8Addr = pu8Buf + u32Start;
        stStream.u32Len = s32ReadLen;
        //stStream.bEndOfFrame = (pstVdecThreadParam->s32StreamMode == VIDEO_MODE_FRAME) ? CVI_TRUE : CVI_FALSE;
        stStream.bEndOfFrame = false;
        stStream.bEndOfStream = bEndOfStream;
        stStream.bDisplay = 1;

SendAgain:
        s32Ret = MAPI_VDEC_SendStream(pstVdecThreadParam->vdec_hdl,&stStream);
        if ((s32Ret != CVI_SUCCESS) && (timeoutcnt < 100)) {
            CVI_LOGE("MAPI_VDEC_SendStream failed,s32Ret:%d.s32UsedBytes:%d,timeoutcnt:%d\n",s32Ret,s32UsedBytes,timeoutcnt);
            usleep(10 * 1000);
            timeoutcnt++;
            goto SendAgain;
        } else {
            timeoutcnt = 0;
            bEndOfStream = CVI_FALSE;
            s32UsedBytes = s32UsedBytes + s32ReadLen + u32Start;
            u64PTS += pstVdecThreadParam->u64PtsIncrease;
            CVI_LOGI("MAPI_VDEC_SendStream over,ret:%d,send_suc_cnt:%d\n",s32Ret,send_suc_cnt++);
        }
    }

    /* send the flag of stream end */
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));
    stStream.bEndOfStream = CVI_TRUE;
    MAPI_VDEC_SendStream(pstVdecThreadParam->vdec_hdl,&stStream);

    CVI_LOGE("\033[0;35m vdec_hdl %p send steam thread return ...  \033[0;39m\n",
            pstVdecThreadParam->vdec_hdl);

    pstVdecThreadParam->bFileEnd = CVI_TRUE;

    fflush(stdout);
    if (pu8Buf != CVI_NULL) {
        free(pu8Buf);
    }
    fclose(fpStrm);
    pthread_exit(0);
    return (void *)CVI_SUCCESS;
}

int main(int argc, char *argv[])
{
    char outputfile[MULTI_ENCODE_MAX][MAX_FILENAME_LEN]={0};
    char *inputfile[MULTI_ENCODE_MAX];
    char *codecname = NULL;
    pthread_t pdecSendTask;
    //pthread_t pdecGetRawTask;
    MAPI_VCODEC_E eVCodeIndex;
    uint32_t ChannelCount = 0;
    int c;
    MAPI_VDEC_THREAD_PARAM_S stVdecThParams;

    MAPI_VDEC_CHN_ATTR_T vdec_attr;
    MAPI_VDEC_HANDLE_T vdec_hdl;
    PAYLOAD_TYPE_E enType;
    MAPI_MEDIA_SYS_ATTR_T sys_attr;

	CVI_MSG_Init();
	CVI_SYS_Init();

    memset(&stVdecThParams,0,sizeof(stVdecThParams));

    opterr = 0;
    // CVI_LOG_INIT();
    while ((c = getopt(argc, argv, "c:i:")) != -1) {
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
            break;
        default:
            printf("Invalid option : %c\n", c);
            print_usage(argv);
            abort ();
        }
    }

    if(!ChannelCount || !codecname ) {
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
    if(ChannelCount> 1) {
        ChannelCount = 1;
        CVI_LOGW("video decode can not support multi channel,only decode the first stream file.\n");
    }

    memset(outputfile,0,sizeof(outputfile));

    for(uint32_t i= 0; i < ChannelCount; i++) {
        strcat(outputfile[i],inputfile[i]);
        strcat(outputfile[i],".");
        strcat(outputfile[i],"yuv");
      //  strcat(outputfile[i],"_decout");
        CVI_LOGI("outputfile[%d]:%s eVCodec:%d\n",i,outputfile[i],eVCodeIndex);
    }

    memcpy(stVdecThParams.cOutFileName,outputfile[0],strlen(outputfile[0]));
    memcpy(stVdecThParams.cInputFileName,inputfile[0],strlen(inputfile[0]));

    vdec_attr.codec = eVCodeIndex;
    enType = gacodec_type[eVCodeIndex];
    MAPI_GetMaxSizeByEncodeType(enType, &vdec_attr.max_width, &vdec_attr.max_height);

    if((PT_JPEG == enType) || (PT_MJPEG == enType)) {
        sys_attr.vb_pool[0].is_frame = false;
        sys_attr.vb_pool[0].vb_blk_size.frame.width  = vdec_attr.max_width;
        sys_attr.vb_pool[0].vb_blk_size.frame.height = vdec_attr.max_height;
        sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
        sys_attr.vb_pool[0].vb_blk_size.size =
            VDEC_GetPicBufferSize(enType,vdec_attr.max_width,vdec_attr.max_height,
                                    PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
        sys_attr.vb_pool[0].vb_blk_num = 3;
        sys_attr.vb_pool_num = 1;
        CHECK_RET(MAPI_Media_Init(&sys_attr));
    }else {
        vdec_attr.max_width = 1920 + 64;
        vdec_attr.max_width = 1088 + 64;
        sys_attr.vb_pool[0].is_frame = false;
        sys_attr.vb_pool[0].vb_blk_size.frame.width  = vdec_attr.max_width;
        sys_attr.vb_pool[0].vb_blk_size.frame.height = vdec_attr.max_height;
        sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
        sys_attr.vb_pool[0].vb_blk_size.size =
            VDEC_GetPicBufferSize(enType,vdec_attr.max_width,vdec_attr.max_height,
                                    PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
        sys_attr.vb_pool[0].vb_blk_num = 3;
        sys_attr.vb_pool_num = 1;
        CHECK_RET(MAPI_Media_Init(&sys_attr));
    }

    CHECK_RET(MAPI_VDEC_InitChn(&vdec_hdl, &vdec_attr));
    stVdecThParams.s32MinBufSize = (vdec_attr.max_width * vdec_attr.max_height * 3) >> 1;
    stVdecThParams.eVDecType = eVCodeIndex;
    stVdecThParams.vdec_hdl = vdec_hdl;
    //pthread_create(&pdecGetRawTask,0,VDEC_GetPicThread,(void *)&stVdecThParams);
    //pthread_detach(pdecGetRawTask);
    //usleep(10*1000);
    pthread_create(&pdecSendTask,0,VdecSendStreamThread,(void *)&stVdecThParams);
    pthread_detach(pdecSendTask);


    getchar();

    return 0;
}