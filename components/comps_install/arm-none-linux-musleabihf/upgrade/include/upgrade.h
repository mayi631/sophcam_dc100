
#ifndef __UPGRADE_H__
#define __UPGRADE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** error code define */
#define UPGRADE_EINTR            1
#define UPGRADE_EPKG_INVALID     2
#define UPGRADE_EPKG_OVERSIZE    3          /* Package oversize */
#define UPGRADE_EIMAGE_OVERSIZE  4          /* Partition image oversize*/
#define UPGRADE_EINVAL           5
#define UPGRADE_EPKG_UNMATCH     6          /* Package un-match to partition*/

#define COMM_STR_LEN 64
#define UPGRADE_MAX_PART_CNT 10
#define COMM_PATH_MAX_LEN 128

#define UPGRADE_EVENT_PROGRESS 1

#define UP_NULL                0L
#define UP_SUCCESS             0
#define UP_FAILURE             (-1)

/** upgrade device information */
typedef struct UPGRADE_DEV_INFO_S {
	char szSoftVersion[COMM_STR_LEN]; /* Software version */
	char szModel[COMM_STR_LEN];       /* Product model */
} UPGRADE_DEV_INFO_S;

/** upgrade package head */
typedef struct UPGRADE_PKG_HEAD_S {
	uint32_t   u32Magic;
	uint32_t   u32Crc;         /* CRC number from HeadVer to end of image-data */
	uint32_t   u32HeadVer;     /* Package head version: 0x00000001 */
	uint32_t   u32PkgLen;      /* Package total length, including head/data */
	uint32_t   bMinusOne;      /* used for fip_spl.bin*/
	char szPkgModel[COMM_STR_LEN]; /* Package model, eg. cv1835_asic_wevb_0002a */
	char szPkgSoftVersion[COMM_STR_LEN];   /* Package version, eg. 1.0.0.0 */
	char PartitionFileName[UPGRADE_MAX_PART_CNT][32]; /*file name for partition, eg. for spinor(partition.xml), 0:fip_spl.bin, 1:yoc.bin */
	char reserved1[704];
	uint32_t   reserved2;
	int32_t  s32PartitionCnt;
	uint32_t   au32PartitionOffSet[UPGRADE_MAX_PART_CNT]; /* Partition offset in upgrade package */
} UPGRADE_PKG_HEAD_S;

/** upgrade event struct */
typedef struct UPGRADE_EVENT_S {
	uint32_t   eventID;
	void *argv;
} UPGRADE_EVENT_S;

int32_t UPGRADE_Init(void);

int32_t UPGRADE_Deinit(void);

int32_t UPGRADE_CheckPkg(const char *pszPkgUrl, const UPGRADE_DEV_INFO_S *pstDevInfo, unsigned char bCheckVer);

int32_t UPGRADE_DoUpgrade(const char *pszPkgUrl);

int32_t UPGRADE_DoUpgradeViaSD(const char *pszPkgUrl, const char *path);

void UPGRADE_RegisterEvent(void (*eventCb)(UPGRADE_EVENT_S *));


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __UPGRADE_H__ */

