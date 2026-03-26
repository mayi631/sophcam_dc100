#ifndef __IMG2IMG_H__
#define __IMG2IMG_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum IMG2IMG_ERROR {
	IMG2IMG_SUCCESS = 0,
	IMG2IMG_INVALID_PARAM = -1,
	IMG2IMG_NETWORK_ERROR = -2,
	IMG2IMG_API_ERROR = -3,
	IMG2IMG_PARSE_ERROR = -4,
	IMG2IMG_BUFFER_TOO_SMALL = -5,
	IMG2IMG_AUTH_ERROR = -6,
	IMG2IMG_FILE_ERROR = -7,
	IMG2IMG_INTERNAL_ERROR = -8,

	IMG2IMG_EC_REQ_INVALID_ARGS = -100,
	IMG2IMG_EC_REQ_MISSING_ARGS = -101,
	IMG2IMG_EC_PARSE_ARGS = -102,
	IMG2IMG_EC_IMAGE_SIZE_LIMITED = -103,
	IMG2IMG_EC_IMAGE_EMPTY = -104,
	IMG2IMG_EC_IMAGE_DECODE_ERROR = -105,
	IMG2IMG_EC_REQ_BODY_SIZE_LIMITED = -106,
	IMG2IMG_EC_RPC_PROCESS = -107,
	IMG2IMG_EC_FS_LEADER_RISK_ERROR = -108,

	IMG2IMG_PRE_IMG_RISK_NOT_PASS = -200,
	IMG2IMG_POST_IMG_RISK_NOT_PASS = -201,
	IMG2IMG_TEXT_RISK_NOT_PASS = -202,
	IMG2IMG_POST_TEXT_RISK_NOT_PASS = -203,
	IMG2IMG_TEXT_NER_IP_BLOCKLIST = -204
};

typedef struct {
	bool add_logo;
	int position;
	int language;
	float opacity;
	const char *text_content;
} img2img_logo_t;

typedef struct {
	const char *model;	     // 模型名称，如 "Qwen-Image-Edit-2509"
	const char *prompt;	     // 文本提示词，可为NULL
	const char *negative_prompt; // 负向提示词，可为NULL
	int seed;		     // 随机种子，-1为随机，默认-1
	bool watermark;		     // 是否添加水印，默认false

	// 以下参数保留用于兼容性，新接口可能不使用
	float cfg;	     // [0.1, 10.0] 默认3.0
	float strength;	     // (0.1, 1.0) 默认0.9925
	int steps;	     // [1, 8] 默认4
	int width;	     // [512,2048]，0由服务端决定
	int height;	     // [512,2048]，0由服务端决定
	bool return_url;     // 默认false
	img2img_logo_t logo; // 水印（旧参数，保留兼容性）
} img2img_params_t;

typedef struct img2img_processor img2img_processor_t;

// 创建处理器：通过 Sophnet easyllm 网关
// 参数：endpoint 形如 "https://www.sophnet.com"
//       project_uuid 项目ID
//       easyllm_id   easyllm服务ID
//       api_key      Bearer Token
img2img_processor_t *img2img_create(const char *endpoint, const char *project_uuid, const char *easyllm_id,
				    const char *api_key);

int img2img_process_file(img2img_processor_t *processor, const char *input_file, const img2img_params_t *params,
			 const char *output_file);

int img2img_process_buffer(img2img_processor_t *processor, const void *input_buffer, size_t input_size,
			   const img2img_params_t *params, const char *output_file);

void img2img_destroy(img2img_processor_t *processor);

img2img_params_t img2img_default_params(void);

const char *img2img_get_error_string(int error_code);

int img2img_validate_params(const img2img_params_t *params);

#ifdef __cplusplus
}
#endif

#endif // __IMG2IMG_H__