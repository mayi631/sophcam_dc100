#include "jpegp.h"
#include <unistd.h>
#include "cvi_vpss.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#define JPEGP_RESIZE_SW

#ifdef JPEGP_RESIZE_SW
static void fail(const char* msg, int err) {
    char buf[256];
    av_strerror(err, buf, sizeof(buf));
    CVI_LOGE("%s: %s\n", msg, buf);
}

// 将内存中一张 JPEG 解码为 AVFrame
static AVFrame* decode_jpeg_from_memory(const uint8_t* data, int size, AVCodecContext** pctx) {
    struct timespec t0, t1;
    uint64_t time_diff = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    const AVCodec* dec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
    if (!dec) {
        fail("find mjpeg decoder", AVERROR_DECODER_NOT_FOUND);
        return NULL;
    }

    AVCodecContext* ctx = avcodec_alloc_context3(dec);
    if (!ctx) {
        fail("alloc dec ctx", AVERROR(ENOMEM));
        return NULL;
    }
    // 对静态图无需额外参数，但可以设置线程
    ctx->thread_count = 1;

    int ret = avcodec_open2(ctx, dec, NULL);
    if (ret < 0) {
        fail("open decoder", ret);
        return NULL;
    }

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        fail("alloc pkt", AVERROR(ENOMEM));
        return NULL;
    }
    // 注意：不拷贝数据，直接引用来避免一次复制
    pkt->data = (uint8_t*)data;
    pkt->size = size;

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fail("alloc frame", AVERROR(ENOMEM));
        av_packet_free(&pkt);
        goto ERR_FREE_PACKET;
    }

    ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        fail("send pkt", ret);
        goto ERR_FREE_FRAME;
    }

    ret = avcodec_receive_frame(ctx, frame);
    if (ret < 0) {
        fail("receive frame", ret);
        goto ERR_FREE_FRAME;
    }

    // 清理 pkt（不释放 data，本示例由调用方管理）
    av_packet_free(&pkt);

    // 返回解码器上下文与帧
    *pctx = ctx;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    time_diff =  (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_nsec - t0.tv_nsec) / 1000000;
    CVI_LOGI("decode time %" PRId64 " ms", time_diff);
    return frame;
ERR_FREE_FRAME:
    av_frame_free(&frame);
ERR_FREE_PACKET:
    av_packet_free(&pkt);
    return NULL;
}

// 缩放到指定尺寸（可选择保持比例）
static AVFrame* scale_frame(const AVFrame* src,
                            int out_w, int out_h,
                            enum AVPixelFormat dst_fmt,
                            int keep_aspect) {
    int in_w = src->width, in_h = src->height;
    int target_w = out_w, target_h = out_h;

    if (keep_aspect) {
        // double sx = (double)out_w / in_w;
        // double sy = (double)out_h / in_h;
        // double s = sx < sy ? sx : sy; // 适配在框内
        // if (s <= 0) s = 1.0;
        // target_w = (int)lrint(in_w * s);
        // target_h = (int)lrint(in_h * s);
        // if (target_w < 1) target_w = 1;
        // if (target_h < 1) target_h = 1;
    }

    struct SwsContext* sws = sws_getContext(
        in_w, in_h, (enum AVPixelFormat)src->format,
        target_w, target_h, dst_fmt,
        // 缩小时可考虑 SWS_AREA；通用用 BICUBIC
        SWS_BICUBIC, NULL, NULL, NULL);
    if (!sws) {
        fail("sws_getContext", AVERROR(EINVAL));
        return NULL;
    }

    AVFrame* dst = av_frame_alloc();
    if (!dst) {
        fail("alloc dst frame", AVERROR(ENOMEM));
        goto ERR_FREE_CTX;
    }
    dst->format = dst_fmt;
    dst->width = target_w;
    dst->height = target_h;
    dst->color_range = AVCOL_RANGE_JPEG;

    int ret = av_frame_get_buffer(dst, 32);
    if (ret < 0) {
        fail("dst frame buffer", ret);
        goto ERR_FREE_FRAME;
    }

    ret = sws_scale(sws,
                    (const uint8_t* const*)src->data, src->linesize,
                    0, in_h,
                    dst->data, dst->linesize);
    if (ret <= 0) {
        fail("sws_scale", ret);
        goto ERR_FREE_FRAME;
    }

    sws_freeContext(sws);
    return dst;
ERR_FREE_FRAME:
    av_frame_free(&dst);
ERR_FREE_CTX:
    sws_freeContext(sws);
    return NULL;
}

// 将 AVFrame 编码为 JPEG，输出到内存（并可写文件）
static int encode_jpeg_to_memory(const AVFrame* frame,
                                 int quality_q, // 1..31，小=好
                                 uint8_t** out_buf, int* out_size) {
    const AVCodec* enc = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!enc) {
        fail("find mjpeg encoder", AVERROR_ENCODER_NOT_FOUND);
        return -1;
    }

    AVCodecContext* ctx = avcodec_alloc_context3(enc);
    if (!ctx) {
        fail("alloc enc ctx", AVERROR(ENOMEM));
        return -1;
    }
    ctx->width = frame->width;
    ctx->height = frame->height;
    ctx->pix_fmt = (enum AVPixelFormat)frame->format;
    ctx->time_base = (AVRational){1, 25};
    ctx->thread_count = 1;

    // 设定质量（qscale）数值越小质量越高
    int q = quality_q;
    if (q < 1) q = 3;
    if (q > 31) q = 31;
    av_opt_set_int(ctx, "q", q, 0);

    // 可选：渐进式 JPEG
    // av_opt_set_int(ctx->priv_data, "progressive", 1, 0);

    int ret = avcodec_open2(ctx, enc, NULL);
    if (ret < 0) {
        fail("open encoder", ret);
        goto ERR_FREE_CTX;
    }

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        fail("alloc pkt", AVERROR(ENOMEM));
        goto ERR_FREE_CTX;
    }

    // pts 可选
    AVFrame* tmp = av_frame_alloc();
    if (!tmp) {
        fail("alloc tmp frame", AVERROR(ENOMEM));
        goto ERR_FREE_PACKET;
    }
    // 直接发送原 frame
    av_frame_unref(tmp);
    // send
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        fail("send frame", ret);
        goto ERR_FREE_FRAME;
    }

    ret = avcodec_receive_packet(ctx, pkt);
    if (ret < 0) {
        fail("receive packet", ret);
        goto ERR_FREE_FRAME;
    }

    // 复制到一块新缓冲返回
    *out_size = pkt->size;
    *out_buf = (uint8_t*)av_malloc(pkt->size);
    if (!*out_buf) {
        fail("alloc out buf", AVERROR(ENOMEM));
        goto ERR_FREE_FRAME;
    }
    memcpy(*out_buf, pkt->data, pkt->size);

    av_packet_free(&pkt);
    avcodec_free_context(&ctx);
    av_frame_free(&tmp);
    return 0;
ERR_FREE_CTX:
    avcodec_free_context(&ctx);
ERR_FREE_PACKET:
    av_packet_free(&pkt);
ERR_FREE_FRAME:
    av_frame_free(&tmp);
    return -1;
}

CVI_S32 JPEGP_Resize(JPEGP_RESIZE_PARAM_S *pstParam){
    CVI_S32 ret = MAPI_SUCCESS;
    AVCodecContext* dec_ctx = NULL;
    AVFrame* src = NULL;
    AVFrame* resized = NULL;
    uint8_t* outbuf = NULL;
    int outsize = 0;

    if(pstParam->src_width == pstParam->dst_width &&
        pstParam->src_height == pstParam->dst_height) {
        CVI_LOGI("same size");
        memcpy(pstParam->pdst, pstParam->psrc, pstParam->src_size);
        pstParam->dst_size = pstParam->src_size;
        src = pstParam->decode_frame;
    }else {
        if(pstParam->is_reuse_decode_frame && pstParam->decode_frame != NULL) {
            src = pstParam->decode_frame;
            CVI_LOGI("reuse");
        }
        if(pstParam->decode_frame == NULL) {
            src = decode_jpeg_from_memory(pstParam->psrc, (int)pstParam->src_size, &dec_ctx);
            pstParam->decode_frame = src;
        }
        if(src == NULL){
            return -1;
        }
        CVI_LOGI("decode(%d): w:%d h:%d format:%s", pstParam->is_reuse_decode_frame, src->width, src->height, av_get_pix_fmt_name((enum AVPixelFormat)src->format));

        enum AVPixelFormat dst_fmt = AV_PIX_FMT_YUVJ420P;
        resized = scale_frame(src, pstParam->dst_width, pstParam->dst_height, dst_fmt, pstParam->aspect_mode);
        if(resized == NULL) {
            av_frame_free(&src);
            return -1;
        }
        CVI_LOGI("scale(out): w:%d h:%d format:%s", resized->width, resized->height, av_get_pix_fmt_name((enum AVPixelFormat)resized->format));

        encode_jpeg_to_memory(resized, 2, &outbuf, &outsize);

        memcpy(pstParam->pdst, outbuf, outsize);
        pstParam->dst_size = outsize;
    }

    if(outbuf != NULL){
        av_free(outbuf);
        outbuf = NULL;
    }
    if(resized != NULL){
        av_frame_free(&resized);
        resized = NULL;
    }

    if(pstParam->is_free_decode_frame && src != NULL) {
        av_frame_free(&src);
        pstParam->decode_frame = NULL;
        CVI_LOGI("free");
    }

    if(dec_ctx != NULL){
        avcodec_free_context(&dec_ctx);
    }

    return ret;
}
#else
static CVI_S32 mjpegParse(void *pData, int dataLen, int *ps32ReadLen, unsigned int *pu32Start) {
    CVI_BOOL bFindStart = CVI_FALSE;
    CVI_BOOL bFindEnd = CVI_FALSE;
    CVI_U8 *pu8Buf = (CVI_U8 *)pData;
    CVI_S32 i = 0;
    CVI_S32 u32Len = 0;

    // FFD8: SOI (Start of Image)
    // FFE0: APPn (Application-specific)
    // FFD9: EOI (End of Image)
    for (i = 0; i < dataLen - 1; i++) {
        if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD8) {
            *pu32Start = i;
            bFindStart = CVI_TRUE;
            i = i + 2;
            break;
        }
    }
    for (; i < dataLen - 3; i++) {
        if ((pu8Buf[i] == 0xFF) && (pu8Buf[i + 1] & 0xF0) == 0xE0) {
            u32Len = (pu8Buf[i + 2] << 8) + pu8Buf[i + 3];
            i += 1 + u32Len;
        } else {
            break;
        }
    }
    for (; i < dataLen - 1; i++) {
        if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD9) {
            bFindEnd = CVI_TRUE;
            break;
        }
    }
    *ps32ReadLen = i + 2;

    if ((bFindStart == CVI_FALSE) || (bFindEnd == CVI_FALSE)) {
        return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

CVI_S32 JPEGP_Resize(JPEGP_RESIZE_PARAM_S *pstParam){
    CVI_S32 ret = MAPI_SUCCESS;
    VB_CONFIG_S vb_config = {0};
    CVI_U32 vb_init_by_jpegp = 0;
    ret = CVI_VB_GetConfig(&vb_config);
    if(ret != CVI_SUCCESS){
        CVI_LOGE("CVI_VB_GetConfig failed");
        return MAPI_ERR_FAILURE;
    }
    CVI_LOGI("vb_config.u32MaxPoolCnt: %d",vb_config.u32MaxPoolCnt);
    if(vb_config.u32MaxPoolCnt <= 0){
        CVI_LOGI("init vb pool by jpegp");
        memset(&vb_config, 0, sizeof(VB_CONFIG_S));
        vb_config.u32MaxPoolCnt = 1;
        vb_config.astCommPool[0].u32BlkSize = 1024;
        vb_config.astCommPool[0].u32BlkCnt = 1;
        vb_config.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
        ret = CVI_VB_SetConfig(&vb_config);
        if(ret != CVI_SUCCESS){
            CVI_LOGE("CVI_VB_SetConfig failed");
            return MAPI_ERR_FAILURE;
        }
        ret = CVI_VB_Init();
        if(ret != CVI_SUCCESS){
            CVI_LOGE("CVI_VB_Init failed");
            return MAPI_ERR_FAILURE;
        }
        vb_init_by_jpegp = 1;
    }

    /* 1. init vdec */
    MAPI_VDEC_CHN_ATTR_T vdec_attr = {0};
    vdec_attr.codec = MAPI_VCODEC_JPG;
    vdec_attr.max_width = pstParam->src_width;
    vdec_attr.max_height = pstParam->src_height;
    MAPI_VDEC_HANDLE_T vdec_hdl = NULL;
    CVI_LOGI("init vdec");
    MAPI_VDEC_SetVBMode(VB_SOURCE_COMMON, 1);
    ret = MAPI_VDEC_InitChn(&vdec_hdl, &vdec_attr);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI vdec init failed");
        return MAPI_ERR_FAILURE;
    }

    /* 2. init venc */
    MAPI_VENC_CHN_ATTR_T venc_attr = {0};
    venc_attr.venc_param.pixel_format = PIXEL_FORMAT_NV12;
    venc_attr.venc_param.codec = MAPI_VCODEC_JPG;
    venc_attr.venc_param.width = pstParam->dst_width;
    venc_attr.venc_param.height = pstParam->dst_height;
    venc_attr.venc_param.rate_ctrl_mode = 4;
    venc_attr.venc_param.bitrate_kbps = 8000;
    venc_attr.venc_param.iqp = 46;
    venc_attr.venc_param.pqp = 46;
    venc_attr.venc_param.maxIqp = 60;
    venc_attr.venc_param.minIqp = 30;
    venc_attr.venc_param.maxQp = 60;
    venc_attr.venc_param.minQp = 30;
    MAPI_VENC_HANDLE_T venc_hdl = NULL;
    CVI_LOGI("init venc");
    ret = MAPI_VENC_InitChn(&venc_hdl, &venc_attr);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI vdec send stream failed");
        return MAPI_ERR_FAILURE;
    }
    MAPI_VENC_StartRecvFrame(venc_hdl, -1);

    /* 3. parse jpeg */
    CVI_S32 jpeg_read_len = 0;
    CVI_U32 jpeg_start = 0;
    ret = mjpegParse(pstParam->psrc, pstParam->src_size, &jpeg_read_len, &jpeg_start);
    if(ret != CVI_SUCCESS){
        CVI_LOGE("mjpegParse failed");
        return MAPI_ERR_FAILURE;
    }
    CVI_LOGI("jpeg_read_len: %d, jpeg_start: %d", jpeg_read_len, jpeg_start);

    /* 4. send stream to vdec */
    VDEC_STREAM_S stStream = {0};
    stStream.u64PTS = 0;
    stStream.pu8Addr = (CVI_U8 *)pstParam->psrc + jpeg_start;
    stStream.u32Len = jpeg_read_len;
    stStream.bEndOfFrame = CVI_FALSE;
    stStream.bEndOfStream = CVI_FALSE;
    stStream.bDisplay = 1;
    ret = MAPI_VDEC_SendStream(vdec_hdl, &stStream);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI vdec send stream failed");
        return MAPI_ERR_FAILURE;
    }

    /* 5. get frame from vdec and send to vproc */
    VIDEO_FRAME_INFO_S frame_dec = {0};
    ret = MAPI_VDEC_GetFrame(vdec_hdl, &frame_dec);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI vproc send frame failed");
        return MAPI_ERR_FAILURE;
    }

    CVI_LOGI("frame_dec.stVFrame.u32Width: %d, frame_dec.stVFrame.u32Height: %d, frame_dec.stVFrame.enPixelFormat: %d\n",
        frame_dec.stVFrame.u32Width, frame_dec.stVFrame.u32Height, frame_dec.stVFrame.enPixelFormat);

    /* 6. init vproc */
    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
        frame_dec.stVFrame.u32Width, frame_dec.stVFrame.u32Height, frame_dec.stVFrame.enPixelFormat,
        pstParam->dst_width, pstParam->dst_height, PIXEL_FORMAT_NV12);
    MAPI_VPROC_HANDLE_T vproc_hdl = NULL;
    vproc_attr.attr_chn[0].stAspectRatio.enMode = pstParam->aspect_mode;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width = pstParam->dst_width;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height = pstParam->dst_height;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32X = 0;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32Y = 0;
    vproc_attr.attr_chn[0].stAspectRatio.bEnableBgColor = CVI_TRUE;
    vproc_attr.attr_chn[0].stAspectRatio.u32BgColor = 0x000000;
    vproc_attr.attr_inp.u8VpssDev = 0;
    vproc_attr.chn_vbcnt[0] = 1;
    CVI_U32 vpss_grp = CVI_VPSS_GetAvailableGrp();
    CVI_LOGI("init vproc, vpss_grp: %d", vpss_grp);
    if (MAPI_VPROC_Init(&vproc_hdl, vpss_grp, &vproc_attr) != 0) {
        CVI_LOGE("MAPI vproc init failed");
        return MAPI_ERR_FAILURE;
    }
    CVI_LOGI("attach vb pool: %d\n", vproc_attr.chn_bindVbPoolID[0]);

    /* 6. send frame to vproc */
    ret = MAPI_VPROC_SendFrame(vproc_hdl, &frame_dec);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI vproc send frame failed");
        return MAPI_ERR_FAILURE;
    }

    // MAPI_VDEC_ReleaseFrame(vdec_hdl, &frame_dec);

    /* 7. get frame from vproc */
    VIDEO_FRAME_INFO_S frame_proc = {0};
    ret = MAPI_VPROC_GetChnFrame(vproc_hdl, 0, &frame_proc);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI vproc get frame failed");
        return MAPI_ERR_FAILURE;
    }

    CVI_LOGI("frame_proc.stVFrame.u32Width: %d, frame_proc.stVFrame.u32Height: %d, frame_proc.stVFrame.enPixelFormat: %d",
        frame_proc.stVFrame.u32Width, frame_proc.stVFrame.u32Height, frame_proc.stVFrame.enPixelFormat);

    /* 8. send frame to venc */
    ret = MAPI_VENC_SendFrame(venc_hdl, &frame_proc);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI venc send frame failed");
        return MAPI_ERR_FAILURE;
    }

    // MAPI_VPROC_ReleaseFrame(vproc_hdl, 0, &frame_proc);

    /* 9. get stream from venc */
    VENC_STREAM_S stream_enc = {0};
    ret = MAPI_VENC_GetStreamTimeWait(venc_hdl, &stream_enc, 3000);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("MAPI venc send frame failed");
        return MAPI_ERR_FAILURE;
    }

    if (stream_enc.u32PackCount <= 0 || stream_enc.u32PackCount > 8) {
        MAPI_VENC_ReleaseStream(venc_hdl, &stream_enc);
        CVI_LOGE("MAPI venc get stream failed");
        return MAPI_ERR_FAILURE;
    }

    CVI_LOGI("stream_enc.pstPack[0].pu8Addr: %p, stream_enc.pstPack[0].u32Len: %d",
        stream_enc.pstPack[0].pu8Addr, stream_enc.pstPack[0].u32Len);

    /* 10. copy stream to dst */
    if(pstParam->pdst != NULL){
        memcpy(pstParam->pdst, stream_enc.pstPack[0].pu8Addr, stream_enc.pstPack[0].u32Len);
        pstParam->dst_size = stream_enc.pstPack[0].u32Len;
    }else{
        pstParam->dst_size = 0;
    }

    /* 11. release */
    MAPI_ReleaseFrame(&frame_dec);
    MAPI_ReleaseFrame(&frame_proc);
    MAPI_VENC_ReleaseStream(venc_hdl, &stream_enc);
    CVI_LOGI("release stream");
    MAPI_VENC_StopRecvFrame(venc_hdl);
    CVI_LOGI("stop recv frame");
    MAPI_VENC_DeinitChn(venc_hdl);
    CVI_LOGI("release venc");
    MAPI_VPROC_Deinit(vproc_hdl);
    CVI_LOGI("release vproc");
    MAPI_VDEC_DeinitChn(vdec_hdl);
    CVI_LOGI("release vdec");

    if(vb_init_by_jpegp){
        memset(&vb_config, 0, sizeof(VB_CONFIG_S));
        CVI_VB_Exit();
        CVI_VB_SetConfig(&vb_config);
    }

    return MAPI_SUCCESS;
}
#endif

CVI_S32 JPEGP_Gen_Thumbnail_And_SubPic(JPEGP_PACKET_PARAM_S *pstParam){
    CVI_U8 JPEG_SOI[2] = {0xFF, 0xD8};
    pstParam->dst_size = 0;

    memcpy(pstParam->pdst, JPEG_SOI, sizeof(JPEG_SOI));
    pstParam->dst_size += sizeof(JPEG_SOI);

    if(pstParam->thumbnail_size < (0xFFFF - 10)){
        CVI_U8 JFXX_header[10] = {
            // 10 = 2+2+5+1
            0xFF, 0xE0,								  // APP0 marker
            (pstParam->thumbnail_size + 8) >> 8, (pstParam->thumbnail_size + 8) & 0xFF, // Length of segment excluding APP0 marker
            0x4A, 0x46, 0x58, 0x58, 0x00,			  // Identifier,
            0x10									  // Thumbnail format, 1 means jpeg
        };
        memcpy(pstParam->pdst + pstParam->dst_size, JFXX_header, sizeof(JFXX_header));
        pstParam->dst_size += sizeof(JFXX_header);

        memcpy(pstParam->pdst + pstParam->dst_size, pstParam->pthumbnail, pstParam->thumbnail_size);
        pstParam->dst_size += pstParam->thumbnail_size;
    }else{
        CVI_LOGE("thumbnail size is too large");
        char JFXX0_header[10] = { // 10 = 2+2+5+1
            0xFF, 0xE0, //APP0 marker
            0x00, 0x08, // Length of segment excluding APP0 marker
            0x4A, 0x46,0x58,0x58, 0x00, // Identifier,
            0x10 // Thumbnail format, 1 means jpeg
        };
        memcpy(pstParam->pdst + pstParam->dst_size, JFXX0_header, sizeof(JFXX0_header));
        pstParam->dst_size += sizeof(JFXX0_header);
    }

    CVI_U8 JPEG_LEN[8] = {
        0xFF, 0xE2, 0x00, 0x06,
        (pstParam->src_size >> 24) & 0xFF, (pstParam->src_size >> 16) & 0xFF,
        (pstParam->src_size >> 8) & 0xFF,  (pstParam->src_size >> 0) & 0xFF
    };
    memcpy(pstParam->pdst + pstParam->dst_size, JPEG_LEN, sizeof(JPEG_LEN));
    pstParam->dst_size += sizeof(JPEG_LEN);

    memcpy(pstParam->pdst + pstParam->dst_size, pstParam->psrc + 2, pstParam->src_size - 2);
    pstParam->dst_size += pstParam->src_size - 2;

    /* write sub pic, not standard jpeg APPn */
    if (pstParam->subpic_size < 0xFFFFFFFF) {
        char JFXX3_header[6] = {
            0xFF, 0xE3, //APP3 marker
            (pstParam->subpic_size >> 24) & 0xFF, (pstParam->subpic_size >> 16) & 0xFF,
            (pstParam->subpic_size >> 8) & 0xFF,  (pstParam->subpic_size >> 0) & 0xFF,
        };
        memcpy(pstParam->pdst + pstParam->dst_size, JFXX3_header, sizeof(JFXX3_header));
        pstParam->dst_size += sizeof(JFXX3_header);

        memcpy(pstParam->pdst + pstParam->dst_size, pstParam->psubpic, pstParam->subpic_size);
        pstParam->dst_size += pstParam->subpic_size;
    } else {
        CVI_LOGE("write sub pic failed\n");
    }

    /* write end */
    CVI_U8 JPEG_EOI[2] = {0xFF, 0xD9};
    memcpy(pstParam->pdst + pstParam->dst_size, JPEG_EOI, sizeof(JPEG_EOI));
    pstParam->dst_size += sizeof(JPEG_EOI);

    return MAPI_SUCCESS;
}

CVI_S32 JPEGP_Gen_Thumbnail_And_SubPic_To_File(JPEGP_PACKET_FILE_PARAM_S *pstParam){
    CVI_S32 ret = MAPI_SUCCESS;
    CVI_U8 *pThumbnail = NULL;
    CVI_U8 *pSubPic = NULL;
    CVI_U8 *pDst = NULL;
    FILE *fpDst = NULL;
    JPEGP_RESIZE_PARAM_S stResizeParam = {0};
    JPEGP_PACKET_PARAM_S stPacketParam = {0};
    CVI_U32 thumbnail_size = 0;
    CVI_U32 subpic_size = 0;
    FILE *fp = fopen(pstParam->src_file_name, "rb");
    if(fp == NULL){
        CVI_LOGE("open file %s failed", (char *)pstParam->src_file_name);
        return MAPI_ERR_FAILURE;
    }
    fseek(fp, 0, SEEK_END);
    CVI_U32 src_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    CVI_U8 *pSrc = (CVI_U8 *)malloc(src_size);
    if(pSrc == NULL){
        CVI_LOGE("malloc src failed");
        return MAPI_ERR_FAILURE;
    }
    fread(pSrc, 1, src_size, fp);
    fclose(fp);

    CVI_LOGI("src_size: %d", src_size);

    pThumbnail = malloc(pstParam->thumbnail_width * pstParam->thumbnail_height * 1.5);
    if(pThumbnail == NULL){
        CVI_LOGE("malloc thumbnail failed");
        ret = MAPI_ERR_FAILURE;
        goto ERR_FREE;
    }

    pSubPic = malloc(pstParam->subpic_width * pstParam->subpic_height * 1.5);
    if(pSubPic == NULL){
        CVI_LOGE("malloc subpic failed");
        ret = MAPI_ERR_FAILURE;
        goto ERR_FREE;
    }

    stResizeParam.psrc = pSrc;
    stResizeParam.src_size = src_size;
    stResizeParam.src_width = pstParam->src_width;
    stResizeParam.src_height = pstParam->src_height;
    stResizeParam.pdst = pThumbnail;
    stResizeParam.dst_size = 0;
    stResizeParam.dst_width = pstParam->thumbnail_width;
    stResizeParam.dst_height = pstParam->thumbnail_height;
    stResizeParam.aspect_mode = ASPECT_RATIO_NONE;
    /* 第一次没有保留的decode frame供使用 */
    stResizeParam.is_reuse_decode_frame = 0;
    stResizeParam.is_free_decode_frame = 0;
    ret = JPEGP_Resize(&stResizeParam);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("JPEGP_Resize failed when thumbnail");
        ret = MAPI_ERR_FAILURE;
        goto ERR_FREE;
    }
    thumbnail_size = stResizeParam.dst_size;
    CVI_LOGI("thumbnail_size: %d", thumbnail_size);

    stResizeParam.psrc = pSrc;
    stResizeParam.src_size = src_size;
    stResizeParam.src_width = pstParam->src_width;
    stResizeParam.src_height = pstParam->src_height;
    stResizeParam.pdst = pSubPic;
    stResizeParam.dst_size = 0;
    stResizeParam.dst_width = pstParam->subpic_width;
    stResizeParam.dst_height = pstParam->subpic_height;
    stResizeParam.aspect_mode = ASPECT_RATIO_AUTO;
    /* 使用上一次的decode frame */
    stResizeParam.is_reuse_decode_frame = 1;
    /* 释放上一次的decode frame */
    stResizeParam.is_free_decode_frame = 1;
    ret = JPEGP_Resize(&stResizeParam);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("JPEGP_Resize failed when subpic");
        ret = MAPI_ERR_FAILURE;
        goto ERR_FREE;
    }
    subpic_size = stResizeParam.dst_size;
    CVI_LOGI("subpic_size: %d", subpic_size);

    pDst = malloc(thumbnail_size + subpic_size + src_size + 1024);
    if(pDst == NULL){
        CVI_LOGE("malloc dst failed");
        ret = MAPI_ERR_FAILURE;
        goto ERR_FREE;
    }

    stPacketParam.psrc = pSrc;
    stPacketParam.src_size = src_size;
    stPacketParam.pthumbnail = pThumbnail;
    stPacketParam.thumbnail_size = thumbnail_size;
    stPacketParam.psubpic = pSubPic;
    stPacketParam.subpic_size = subpic_size;
    stPacketParam.pdst = pDst;
    stPacketParam.dst_size = 0;
    ret = JPEGP_Gen_Thumbnail_And_SubPic(&stPacketParam);
    if(ret != MAPI_SUCCESS){
        CVI_LOGE("JPEGP_Gen_Thumbnail_And_SubPic failed");
        ret = MAPI_ERR_FAILURE;
        goto ERR_FREE;
    }
    CVI_LOGI("dst_size: %d", stPacketParam.dst_size);

    fpDst = fopen(pstParam->dst_file_name, "wb");
    if(fpDst == NULL){
        CVI_LOGE("open dst file failed");
        ret = MAPI_ERR_FAILURE;
        goto ERR_FREE;
    }
    fwrite(pDst, 1, stPacketParam.dst_size, fpDst);
    fclose(fpDst);
ERR_FREE:
    if(pSrc != NULL){
        free(pSrc);
        pSrc = NULL;
    }
    if(pThumbnail != NULL){
        free(pThumbnail);
        pThumbnail = NULL;
    }
    if(pSubPic != NULL){
        free(pSubPic);
        pSubPic = NULL;
    }
    if(pDst != NULL){
        free(pDst);
        pDst = NULL;
    }

    return ret;
}