#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <cvi_type.h>
#include <cvi_audio.h>
#include <cvi_comm_aio.h>
#include "mapi_ao.h"
#include "cvi_log.h"
#include "mapi_aenc.h"

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
typedef struct _AO_Chn_Info_ {
    MAPI_AO_ATTR_S stAoAttr;
    MAPI_AO_HANDLE_T AoHdl;
    CVI_S32 AoChn;
    char *pBuffer;
    int buffersize;
    FILE * pfile;
    int ChnSample_rate;
    int ChnCnt;
}AO_CHN_Info;
#define STRING_NAME_LEN (128)
#define AO_CHN_COUNT_MAX 8
int playing = 1;
char codec_name[][STRING_NAME_LEN]={"aac","g711a","g711u"};

void sigint_handler(int sig )
{
    playing = 0;
}

// int dumpdata(char *filename,char *buf,unsigned int len)
// {
//     printf("enturn dumpdata\n");
//     FILE *fp;
//     if(filename == NULL)
//     {
//         return -1;
//     }

//     fp = fopen(filename,"ab+");

//     fwrite(buf,1,len,fp);
//     fclose(fp);
//     return 0;
// }

static void print_usage(char * const *argv)
{
    printf("Usage: %s -b 1 -f g711u -r 8000 -p 320 -c 2 -i inputfile1 inputfile2 ...\n", argv[0]);
    printf("-b : 1 or 0 (bind or not bind Adec)\n");
    printf("-f : g711u g711a aac\n");
    printf("-r : 8000 16000 48000 (sample rate)\n");
    printf("-p : period_size \n");
    printf("-c : 2 0r 1 (Channles)\n");
    printf("-i : inputfile name \n");
    printf("-R : channel samplerate\n");
    printf("-C : channel cnt \n");

    printf("all file have to the same of parameter\n");
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
    return -1;
}

void audio_close_wavefile(FILE *pfile)
{
    if(pfile)
        fclose(pfile);
}

FILE *audio_open_wavfile(const char *filename,int *channels,int *sample_rate)
{
    FILE *file;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
    int more_chunks = 1;

    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr,"Unable to open file '%s'\n", filename);
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
            *sample_rate = chunk_fmt.sample_rate;
            *channels = chunk_fmt.num_channels;
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
    return file;
}
#if 0
int  Ao_GetEncodeDataCB(MAPI_AO_HANDLE_T AoHdl,AUDIO_STREAM_S* pAStreamData, void *pPrivateData)
{
    AO_CB_INFO *pstAoCbInfo = (AO_CB_INFO *)pPrivateData;
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    int u32ReadLen ,ret;
    pAStreamData->pStream = (unsigned char *)pstAoCbInfo->pBuffer;
    //u32ReadLen = fread(pAStreamData->pStream, 1, 640, pstAoCbInfo->fp);//------------
    u32ReadLen = fread(pAStreamData->pStream, 1, pstAoCbInfo->u32PtNumPerFrm, pstAoCbInfo->fp);//------------
    if (u32ReadLen ==  0) {
        ret = MAPI_ADEC_SendEndOfStream(et->AdecHdl);
        playing = 0;
    }
    printf("pstAoCbInfo->period_size = %d\n",pstAoCbInfo->u32PtNumPerFrm);
    pAStreamData->u32Len = u32ReadLen;
    ret = MAPI_ADEC_SendStream(et->AdecHdl,pAStreamData, CVI_TRUE);
    if (ret != CVI_SUCCESS) {
        CVI_LOGE("MAPI_ADEC_SendStream send failed,ret:%d.\n",ret);
        return -1;
    }
    return MAPI_SUCCESS;
}
#endif
static CVI_BOOL _checkname_iswav(char *infilename)
{
    CVI_S32 s32InputFileLen = 0;

    s32InputFileLen = strlen(infilename);

    if (s32InputFileLen == 0) {
        printf("No Input File Name..force return\n");
        return 0;
    }

    if (infilename[s32InputFileLen-4] == '.' &&
    (infilename[s32InputFileLen-3] == 'W' || infilename[s32InputFileLen-3] == 'w') &&
    (infilename[s32InputFileLen-2] == 'A' || infilename[s32InputFileLen-2] == 'a') &&
    (infilename[s32InputFileLen-1] == 'V' || infilename[s32InputFileLen-1] == 'v')) {
        printf("Enter wav file\n");
        return CVI_TRUE;
    } else
        return CVI_FALSE;

}
CVI_VOID* AudioAoOtherChnPlay(CVI_CHAR * argcs)
{

    int s32Ret;
    AUDIO_FRAME_S stFrame;
    int s32FrameBytes;
    AO_CHN_Info *pstChnInfo =(AO_CHN_Info *)argcs;

    signal(SIGINT, sigint_handler);
    signal(SIGHUP, sigint_handler);
    signal(SIGTERM, sigint_handler);
    printf("enter AudioAoOtherChnPlay\n");



    if (pstChnInfo->ChnSample_rate != (int)pstChnInfo->stAoAttr.enSampleRate) {
        s32Ret = MAPI_AO_EnableReSmp(pstChnInfo->AoHdl, pstChnInfo->AoChn,
            (AUDIO_SAMPLE_RATE_E)pstChnInfo->ChnSample_rate);
        if(s32Ret){
            CVI_LOGE("AoReSamp faile AoChn:%d,chnSr:%d, devSr:%d\n",pstChnInfo->AoChn,
                pstChnInfo->ChnSample_rate, pstChnInfo->stAoAttr.enSampleRate);
            return NULL;
        }
    }

    s32Ret = MAPI_AO_Start(pstChnInfo->AoHdl,pstChnInfo->AoChn);
    if(s32Ret){
        CVI_LOGE("MAPI_AO_Start() failed\n");
        goto ERR2;
    }
    if(MAPI_AO_SetVolume(pstChnInfo->AoHdl,10)){
        CVI_LOGE("CVI_AO_SetVolume() ERR\n");
        playing = 0;
    }
    s32FrameBytes =pstChnInfo->buffersize;
    stFrame.u64VirAddr[0] = malloc(s32FrameBytes);
    while(playing) {//
        memset(pstChnInfo->pBuffer,0,s32FrameBytes);
        if( fread(pstChnInfo->pBuffer,1,s32FrameBytes,pstChnInfo->pfile) > 0) {
            memcpy(stFrame.u64VirAddr[0],pstChnInfo->pBuffer,s32FrameBytes);
            stFrame.u32Len = s32FrameBytes/(2*pstChnInfo->stAoAttr.channels);//16bit 2chn,
            stFrame.u64TimeStamp = 0;
            stFrame.enSoundmode = (pstChnInfo->ChnCnt==2)?1:0;
            stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
            s32Ret =MAPI_AO_SendFrame(pstChnInfo->AoHdl,pstChnInfo->AoChn,&stFrame,1000);
            if(s32Ret){
                CVI_LOGE("[error] Thread 2 MAPI_AO_SendFrame err [%d]\n",__LINE__);
                goto ERR1;
            }
            printf("[AudioAoOtherChnPlay] stFrame.u32Len =%d",stFrame.u32Len);
        } else {
            break;
        }
    }

    if (pstChnInfo->ChnSample_rate != (int)pstChnInfo->stAoAttr.enSampleRate)
        MAPI_AO_DisableReSmp(pstChnInfo->AoHdl,pstChnInfo->AoChn);

    audio_close_wavefile(pstChnInfo->pfile);
    MAPI_AO_Stop(pstChnInfo->AoHdl,pstChnInfo->AoChn);
ERR1:
    free(stFrame.u64VirAddr[0]);
ERR2:
    return pstChnInfo->pBuffer;
}
int main(int argc,char *argv[])
{
    int s32Ret=MAPI_SUCCESS;
    MAPI_AO_HANDLE_T AoHdl;
    int num_framebytes;
    int ret;
    int bBind = 0;
    int s32FrameBytes;
    FILE *pfile[AO_CHN_COUNT_MAX];
    FILE *audio_file =NULL;
    char *inputfile[AO_CHN_COUNT_MAX];
    char *codecname =NULL;
    AUDIO_FRAME_S stFrame;
    MAPI_ADEC_HANDLE_T AdecHdl;
    MAPI_ADEC_ATTR_S stAdecAttr;
    AUDIO_SAMPLE_RATE_E sample_rate = 8000;
    uint32_t channels = 2;
    uint32_t period_size= 320;
    CVI_S32 AoChn =0;
    int filecount = 1;
    int u32ReadLen;
    int buflen_byte;
    int ChannelCnt = 2;
    int ChnSampleRate = 8000;

    AUDIO_STREAM_S stAdecStream;
    while((ret = getopt(argc,argv,"b:f:r:p:c:i:R:C:"))!= -1)
    {
        switch(ret) {
            case 'b':
                bBind = atoi(optarg);
                break;
            case 'f':
                codecname= optarg;
                break;
            case 'r':
                sample_rate= atoi(optarg);
                break;
            case 'p':
                period_size= atoi(optarg);
                break;
            case 'c':
                channels= atoi(optarg);
                break;
            case 'R':
                ChnSampleRate = atoi(optarg);
                break;
            case 'C':
                ChannelCnt = atoi(optarg);
                break;
            case 'i':         //ptind=13    1
                    inputfile[0]= optarg;
                    int k=0;
                    printf("-i optind:%d\n",optind);
                    while(k < AO_CHN_COUNT_MAX -1 && argv[optind+k] && (strcmp(argv[optind+k],"-b")!= 0) ) {
                        inputfile[k+1] = argv[optind+k];
                        printf("optind=%d,argv[%d]:%s\n",optind,optind+k,inputfile[k+1]);
                        k++;
                    }
                    filecount+=k;
                    printf("final inputfilke count = %d\n",filecount);
                break;
            default:
                print_usage(argv);
                break;
        }
    }
    signal(SIGINT, sigint_handler);
    signal(SIGHUP, sigint_handler);
    signal(SIGTERM, sigint_handler);

    if(strcmp(codecname,"aac") == 0)
        period_size = 1024;

    MAPI_AO_ATTR_S stAoAttr;
    stAoAttr.enSampleRate=sample_rate;//8000;
    stAoAttr.channels=channels;//2;
    stAoAttr.u32PtNumPerFrm=320;//320;

    if(!bBind) {//no bind
       for(int i=0;i < filecount; i++) {
        if (_checkname_iswav(inputfile[i]))
            pfile[i] = audio_open_wavfile(inputfile[i],(int *)&stAoAttr.channels,(int *)&stAoAttr.enSampleRate);
        else
            pfile[i] = fopen(inputfile[i],"rb");

        if(pfile[i] == NULL) {
             CVI_LOGE("OPEN FILE ERR\n");
             return -1;
         }
         printf("[cviaudio] Open file %s\n",inputfile[i]);
       }
    } else {//bind
        printf("bind\n");
        audio_file = fopen(inputfile[0],"rb");
        if (!audio_file) {
            CVI_LOGE("Unable2 to open file '%s'\n", inputfile[0]);
            return -1;
        }
    }


    if(filecount > 2) {
        printf("mapi_aplay_demo only support two file to play\n");
        return -1;
    }
    s32FrameBytes = stAoAttr.u32PtNumPerFrm * stAoAttr.channels * 2;

    if (bBind)
       buflen_byte = period_size;
    else
        buflen_byte = s32FrameBytes;

    char *pBuffer = malloc(buflen_byte);//2ch 16bit 160 samples
    if (!pBuffer) {
       CVI_LOGE("unable to allocate s32FrameBytes\n");
        return -1;
    }
    stFrame.u64VirAddr[0] =  malloc(buflen_byte);


    s32Ret = MAPI_AO_Init(&AoHdl,&stAoAttr);
    if(s32Ret){
        CVI_LOGE("MAPI_AO_Init() failed\n");
        return s32Ret;
    }
    pthread_t tid;
    char*p;
    if (!bBind && filecount > 1) {
        AO_CHN_Info stAoChnInfo;
        stAoChnInfo.AoHdl =AoHdl;
        stAoChnInfo.AoChn = 1;
        stAoChnInfo.pfile = pfile[1];
        stAoChnInfo.buffersize = s32FrameBytes;
        stAoChnInfo.ChnCnt = ChannelCnt;
        stAoChnInfo.ChnSample_rate = ChnSampleRate;
        stAoChnInfo.pBuffer = malloc(stAoChnInfo.buffersize);
        memcpy(&stAoChnInfo.stAoAttr,&stAoAttr,sizeof(MAPI_AO_ATTR_S));
        pthread_create(&tid,NULL,(void *)AudioAoOtherChnPlay,(void *)&stAoChnInfo);
    }

    s32Ret = MAPI_AO_Start(AoHdl,AoChn);
    if(s32Ret){
        CVI_LOGE("MAPI_AO_Start() failed\n");
        return s32Ret;
    }
    if(MAPI_AO_SetVolume(AoHdl,10)){
        CVI_LOGE("CVI_AO_SetVolume() ERR\n");
        playing = 0;
    }


    if (ChnSampleRate != (int)stAoAttr.enSampleRate) {
        s32Ret = MAPI_AO_EnableReSmp(AoHdl, AoChn, (AUDIO_SAMPLE_RATE_E)ChnSampleRate);
        if(s32Ret){
            CVI_LOGE("ao resample faile chnSr:%d, devSr:%d\n", ChnSampleRate, stAoAttr.enSampleRate);
            return -1;
        }
    }

    if(bBind) {
        if (ChannelCnt != (int)stAoAttr.channels) {
            CVI_LOGE("bind mode not support .please use get_use_mode\n");
            return -1;
        }
        memset(&stAdecAttr,0,sizeof(MAPI_ADEC_ATTR_S));
        stAdecAttr.enSamplerate = sample_rate;
        stAdecAttr.enSoundmode =channels - 1;
        stAdecAttr.frame_size = period_size;
        stAdecAttr.enAdecFormat =GetCodecIndex(codecname);
        stAdecAttr.enMode = ADEC_MODE_STREAM;
        MAPI_ADEC_Init(&AdecHdl,&stAdecAttr);
        MAPI_AO_BindAdec(0,0,0);

    }

    while(playing && bBind) { //bind
        stAdecStream.pStream =(CVI_U8  *)pBuffer;

        u32ReadLen = fread(stAdecStream.pStream, 1,period_size, audio_file);
        if (u32ReadLen ==  0) {
            ret = MAPI_ADEC_SendEndOfStream(AdecHdl);
            playing = 0;
        }

        stAdecStream.u32Len = u32ReadLen;
        ret = MAPI_ADEC_SendStream(AdecHdl,&stAdecStream, CVI_TRUE);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("MAPI_ADEC_SendStream send failed,ret:%d.\n",ret);
            return -1;
        }

        //usleep(20*1000);
    }
    while(playing && (!bBind))
    {
        memset(pBuffer,0,s32FrameBytes);
        num_framebytes=fread(pBuffer,1,s32FrameBytes,pfile[0]);
        if(num_framebytes>0){
            memcpy(stFrame.u64VirAddr[0],pBuffer,s32FrameBytes);
            stFrame.u32Len = s32FrameBytes/(stAoAttr.channels*2);
            stFrame.u64TimeStamp = 0;
            stFrame.enSoundmode = (ChannelCnt==2)?1:0;
            stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;

            s32Ret = MAPI_AO_SendFrame(AoHdl,AoChn,&stFrame,1000);
            if(s32Ret){
                CVI_LOGE("MAPI_AO_SendFrame failed!\n");
                return s32Ret;
            }
            printf("[main] stFrame.u32Len =%d\n",stFrame.u32Len);
        }else {
            break;
        }

    }
    if(bBind) {
        MAPI_AO_UnbindAdec(0,0,0);

        fclose(audio_file);
        MAPI_ADEC_Deinit(AdecHdl);

    }else {
        audio_close_wavefile(pfile[0]);
        if (ChnSampleRate != (int)stAoAttr.enSampleRate)
            MAPI_AO_DisableReSmp(AoHdl, AoChn);
        if(filecount > 1) {
            pthread_join(tid,(void **)&p);
            free(p);
        }

    }
        free(pBuffer);
        MAPI_AO_Stop(AoHdl,AoChn);
        MAPI_AO_Deinit(AoHdl);
    TEST_PASS;
    return 0;
}

