/**
 * @file jpegp.h
 * @brief JPEG 处理模块接口定义，提供缩略图生成、子图合成、图像缩放等功能。
 */

#ifndef __JPEGP_H__
#define __JPEGP_H__
#include "mapi.h"

/**
 * @brief JPEG 缩放参数结构体
 * @details 用于配置 JPEG 图像缩放操作的输入输出参数，支持解码帧复用。
 */
typedef struct _JPEGP_RESIZE_PARAM_S {
    const CVI_VOID* psrc; ///< [in]  输入 JPEG 数据源指针
    CVI_U32 src_size; ///< [in]  输入 JPEG 数据大小（字节）
    CVI_U32 src_width; ///< [in]  输入图像宽度（像素）
    CVI_U32 src_height; ///< [in]  输入图像高度（像素）
    CVI_VOID* pdst; ///< [out] 输出缩放数据缓冲区指针
    CVI_U32 dst_size; ///< [out] 输出缩放数据大小（字节）
    CVI_U32 dst_width; ///< [in]  输出图像宽度（像素）
    CVI_U32 dst_height; ///< [in]  输出图像高度（像素）
    CVI_U32 aspect_mode; ///< [in]  宽高比模式: 0-不使用, 1-自动适配
    CVI_U32 is_reuse_decode_frame; ///< [in]  是否复用已解码的帧（避免重复解码）
    CVI_U32 is_free_decode_frame; ///< [in]  是否在完成后释放解码帧
    CVI_VOID* decode_frame; ///< [in/out] 解码帧指针（复用时传入，释放时置空）
} JPEGP_RESIZE_PARAM_S;

/**
 * @brief JPEG 打包参数结构体
 * @details 用于将缩略图、子图和原始 JPEG 数据打包成复合 JPEG 格式。
 */
typedef struct _JPEGP_PACKET_PARAM_S {
    const CVI_VOID* psrc; ///< [in]  原始 JPEG 数据源指针
    CVI_U32 src_size; ///< [in]  原始 JPEG 数据大小（字节）
    const CVI_VOID* psubpic; ///< [in]  子图数据指针（可选，可为 NULL）
    CVI_U32 subpic_size; ///< [in]  子图数据大小（字节）
    const CVI_VOID* pthumbnail; ///< [in]  缩略图数据指针（可选，可为 NULL）
    CVI_U32 thumbnail_size; ///< [in]  缩略图数据大小（字节）
    CVI_VOID* pdst; ///< [out] 打包输出缓冲区指针
    CVI_U32 dst_size; ///< [out] 打包输出数据大小（字节）
} JPEGP_PACKET_PARAM_S;

/**
 * @brief JPEG 打包文件参数结构体
 * @details 从文件读取 JPEG，生成缩略图和子图，然后打包输出到目标文件。
 */
typedef struct _JPEGP_PACKET_FILE_PARAM_S {
    const CVI_VOID* src_file_name; ///< [in]  源 JPEG 文件路径
    CVI_U32 src_width; ///< [in]  源图像宽度（像素）
    CVI_U32 src_height; ///< [in]  源图像高度（像素）
    CVI_U32 subpic_width; ///< [in]  子图目标宽度（像素）
    CVI_U32 subpic_height; ///< [in]  子图目标高度（像素）
    CVI_U32 thumbnail_width; ///< [in]  缩略图目标宽度（像素）
    CVI_U32 thumbnail_height; ///< [in]  缩略图目标高度（像素）
    const CVI_VOID* dst_file_name; ///< [in]  目标输出文件路径
} JPEGP_PACKET_FILE_PARAM_S;

/**
 * @brief 缩放 JPEG 图像
 * @param[in] pstParam 缩放参数指针，包含输入输出数据和配置
 * @return CVI_S32 成功返回 MAPI_SUCCESS，失败返回错误码
 */
CVI_S32 JPEGP_Resize(JPEGP_RESIZE_PARAM_S* pstParam);

/**
 * @brief 生成包含缩略图和子图的复合 JPEG 数据
 * @param[in] pstParam 打包参数指针
 * @return CVI_S32 成功返回 MAPI_SUCCESS，失败返回错误码
 * @note 生成的 JPEG 结构: SOI + APP0(JFXX+缩略图) + APP2(原图数据) + APP3(子图) + EOI
 */
CVI_S32 JPEGP_Gen_Thumbnail_And_SubPic(JPEGP_PACKET_PARAM_S* pstParam);

/**
 * @brief 从文件生成包含缩略图和子图的复合 JPEG 文件
 * @param[in] pstParam 文件打包参数指针
 * @return CVI_S32 成功返回 MAPI_SUCCESS，失败返回错误码
 */
CVI_S32 JPEGP_Gen_Thumbnail_And_SubPic_To_File(JPEGP_PACKET_FILE_PARAM_S* pstParam);

#endif
