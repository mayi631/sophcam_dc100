#ifndef __MQ_H__
#define __MQ_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MQ_SUCCESS             ((int)(0))
#define MQ_ERR_FAILURE         ((int)(-1001))
#define MQ_ERR_NOMEM           ((int)(-1002))
#define MQ_ERR_TIMEOUT         ((int)(-1003))
#define MQ_ERR_AGAIN           ((int)(-1004))

// Predefined CLIENT ID for inter-process MQ
#define MQ_CLIENT_ID_INVALID   (0)
#define MQ_CLIENT_ID_SCS       (1)
#define MQ_CLIENT_ID_CLI       (50)
#define MQ_CLIENT_ID_SVC_0     (100)
#define MQ_CLIENT_ID_SVC_1     (101)
#define MQ_CLIENT_ID_SVC_2     (102)
#define MQ_CLIENT_ID_SVC_3     (103)
#define MQ_CLIENT_ID_APP_0     (200)
#define MQ_CLIENT_ID_APP_1     (201)
#define MQ_CLIENT_ID_APP_2     (202)
#define MQ_CLIENT_ID_APP_3     (203)
#define MQ_CLIENT_ID_USER_0    (300)
#define MQ_CLIENT_ID_USER_1    (301)
#define MQ_CLIENT_ID_USER_2    (302)
#define MQ_CLIENT_ID_USER_3    (303)
#define MQ_CLIENT_ID_MAX       (0xffff)

typedef uint32_t MQ_ID_t;
#define MQ_ID(client, channel)     (((client) << 16) | (channel))
#define MQ_ID_GET_CLIENT(id)       (((id) >> 16) & 0xFFFF)
#define MQ_ID_GET_CHANNEL(id)      ((id) & 0xFFFF)

#define MQ_MSG_PAYLOAD_LEN     (512)
#define MQ_QUEUE_SIZE          (32)

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

typedef struct MSG_ACK_s {
    char  ackmsg[MQ_MSG_PAYLOAD_LEN];
    int32_t result_len;
    int32_t status;
} __attribute__((__packed__)) MSG_ACK_t;

typedef struct MQ_MSG_s {
    MQ_ID_t target_id;
    int32_t arg1;
    int32_t arg2;
    int16_t seq_no;
    int16_t len;
    uint64_t crete_time;
    uint32_t needack;
    int16_t client_id;
    char payload[MQ_MSG_PAYLOAD_LEN];
} __attribute__((__packed__)) MQ_MSG_S;
#define MQ_MSG_HEADER_LEN      (offsetof(struct MQ_MSG_s, payload))

struct MQ_ENDPOINT_S;
typedef struct MQ_ENDPOINT_S MQ_ENDPOINT_t;
typedef struct MQ_ENDPOINT_S *MQ_ENDPOINT_HANDLE_t;

typedef int (*MQ_RECV_CB_t)(
    MQ_ENDPOINT_HANDLE_t      ep,
    MQ_MSG_S                  *msg,
    void                          *ep_arg);

typedef struct {
    const char                    *name;
    MQ_ID_t                    id;
    MQ_RECV_CB_t               recv_cb;
    void                          *recv_cb_arg;
} MQ_ENDPOINT_CONFIG_S;

int MQ_CreateEndpoint(
    MQ_ENDPOINT_CONFIG_S      *config,
    MQ_ENDPOINT_HANDLE_t      *ep);

int MQ_DestroyEndpoint(
	MQ_ENDPOINT_HANDLE_t       ep);

int MQ_SendRAW(
    MQ_MSG_S                  *msg);

int MQ_Send(
    MQ_ID_t                    target_id,
    int32_t                        arg1,
    int32_t                        arg2,
    int16_t                        seq_no,
    char                          *payload,
    int16_t                        payload_len);

int MQ_SendAck(MQ_ENDPOINT_HANDLE_t ep,
    MSG_ACK_t   *ack_msg, int16_t client_id);
int MQ_SendNeedAck(
    MQ_ID_t                    target_id,
    int32_t                        arg1,
    int32_t                        arg2,
    int16_t                        seq_no,
    char                          *payload,
    int16_t                        payload_len,
    MSG_ACK_t                      *ack_msg,
    int16_t                         client_id,
    int32_t                         timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
