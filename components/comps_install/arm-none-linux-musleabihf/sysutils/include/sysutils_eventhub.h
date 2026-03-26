#ifndef EVENTHUB_H
#define EVENTHUB_H
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define MSG_PAYLOAD_LEN 128

typedef uint32_t TOPIC_ID;
typedef void *MW_PTR;

#define EVENTHUB_SUBSCRIBE_NAME_LEN 16

typedef struct ps_msg_s {
    uint32_t topic;  // Message topic
    int32_t arg1;
    int32_t arg2;
    int32_t s32Result;
    uint64_t u64CreateTime;
    uint8_t aszPayload[MSG_PAYLOAD_LEN];
} EVENT_S;

typedef struct {
    char asName[EVENTHUB_SUBSCRIBE_NAME_LEN];
    void *argv;
    int32_t (*new_msg_cb)(void *argv, EVENT_S *msg);
    bool sync;
} EVENTHUB_SUBSCRIBER_S;

/**
 * @brief EVENTHUB_Init init EVENTHUB system
 *
 * @param none
 *
 * @return none
 */
int32_t EVENTHUB_Init(void);
/**
 * @brief EVENTHUB_DeInit deinit EVENTHUB system
 *
 * @param none
 *
 * @return none
 */
int32_t EVENTHUB_DeInit(void);

/**
 * @brief EVENTHUB_RegisterTopic register a topic in EVENTHUB system(not a must
 * operation)
 *
 * @param TOPIC_ID topic id
 *
 * @return 0 success / -1 failed
 */

int32_t EVENTHUB_RegisterTopic(TOPIC_ID topic);

/**
 * @brief EVENTHUB_UnRegisterTopic unregister a topic in EVENTHUB system(not a
 * must operation)
 *
 * @param TOPIC_ID topic id
 *
 * @return 0 success / -1 failed
 */
int32_t EVENTHUB_UnRegisterTopic(TOPIC_ID topic);

/**
 * @brief EVENTHUB_GetEventHistory get the latest msg of  published on the
 * topic
 *
 * @param TOPIC_ID topic id, EVENT_S msg point
 *
 * @return 0 success / -1 failed
 */
int32_t EVENTHUB_GetEventHistory(TOPIC_ID topic, EVENT_S *msg);

/**
 * @brief EVENTHUB_CreateSubscriber create a subscriber
 *
 * @param EVENTHUB_SUBSCRIBER_S *pstSubscriber  SUBSCRIBER struct point
 *        MW_PTR *ppSubscriber hanlder of pstSubsciber
 *
 * @return 0 success / -1 failed
 */
int32_t EVENTHUB_CreateSubscriber(EVENTHUB_SUBSCRIBER_S *pstSubscriber,
                                MW_PTR *ppSubscriber);

/**
 * @brief EVENTHUB_DestroySubscriber destroy a subscriber
 *
 * @param MW_PTR *ppSubscriber hanlder of pstSubsciber
 *
 * @return 0 success / -1 failed
 */
int32_t EVENTHUB_DestroySubscriber(MW_PTR ppSubscriber);

/**
 * @brief EVENTHUB_Subcribe subscriber to the topic
 *
 * @param MW_PTR *pSubscriber hanlder of pstSubscriber, TOPIC_ID topic
 * subscribed topic
 *
 * @return 0 success / -1 failed
 */
int32_t EVENTHUB_Subcribe(MW_PTR pSubscriber, TOPIC_ID topic);

/**
 * @brief EVENTHUB_UnSubcribe unsubscriber to the subscribed topic
 *
 * @param MW_PTR *pSubscriber hanlder of pstSubscriber, TOPIC_ID topic
 * subscribed topic
 *
 * @return 0 success / -1 failed
 */
int32_t EVENTHUB_UnSubcribe(MW_PTR pSubscriber, TOPIC_ID topic);

/**
 * @brief EVENTHUB_Publish publish the event structure
 *
 * @param EVENT_S *pEvent the event structrue to be published
 *
 * @return 0 success / -1 failed
 */
int32_t EVENTHUB_Publish(EVENT_S *pEvent);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
