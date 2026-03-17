/**
* @file    gpsmng.h
* @brief   product gps interface
*
*
* @author    cardv team
* @date      2021/6/20
* @version

*/
#ifndef __GPSMNG_H__
#define __GPSMNG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "appcomm.h"

/** error code define */
#define GPSMNG_EINVAL                   APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_EINVAL)/**<param error */
#define GPSMNG_ENOTINIT                 APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ENOINIT)/**<Not inited */
#define GPSMNG_EINITIALIZED             APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_EINITIALIZED)/**<Already Initialized */
#define GPSMNG_EINTER                   APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_EINTER)/**<Internal error */
#define GPSMNG_EREGISTEREVENT           APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ERRNO_CUSTOM_BOTTOM)/**<thread creat or join error*/
#define GPSMNG_EREGISTER                APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ERRNO_CUSTOM_BOTTOM + 1)/**<register error*/
#define GPSMNG_EUREGISTER               APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ERRNO_CUSTOM_BOTTOM + 2)/**<uregister error*/
#define GPSMNG_ESTART                   APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ERRNO_CUSTOM_BOTTOM + 3)/**<start error*/
#define GPSMNG_EALREADYSTART            APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ERRNO_CUSTOM_BOTTOM + 4)/**<start error*/
#define GPSMNG_EDEINIT                  APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ERRNO_CUSTOM_BOTTOM + 5)/**<deinit error*/
#define GPSMNG_EGETDATA                 APP_APPCOMM_ERR_ID(APP_MOD_GPSMNG, APP_ERRNO_CUSTOM_BOTTOM + 6)/**<get data error*/

#define GPSMNG_CALLBACK_MAX_NUM (4)
#define GPSMNG_GSV_MAX_MSG_NUM (4)
#define NMEA_SATINPACK      (4)
#define NMEA_MAXSAT         (12)

/**
 * Date and time data
 * @see nmea_time_now
 */
typedef struct _nmeaTIME
{
    int32_t     year;       /**< Years since 1900 */
    int32_t     mon;        /**< Months since January - [0,11] */
    int32_t     day;        /**< Day of the month - [1,31] */
    int32_t     hour;       /**< Hours since midnight - [0,23] */
    int32_t     min;        /**< Minutes after the hour - [0,59] */
    int32_t     sec;        /**< Seconds after the minute - [0,59] */
    int32_t     hsec;       /**< Hundredth part of second - [0,99] */

} nmeaTIME;

/**
 * GGA packet information structure (Global Positioning System Fix Data)
 */
typedef struct _nmeaGPGGA
{
    nmeaTIME utc;       /**< UTC of position (just time) */
	double  lat;        /**< Latitude in NDEG - [degree][min].[sec/60] */
    char    ns;         /**< [N]orth or [S]outh */
	double  lon;        /**< Longitude in NDEG - [degree][min].[sec/60] */
    char    ew;         /**< [E]ast or [W]est */
    int32_t     sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
	int32_t     satinuse;   /**< Number of satellites in use (not those in view) */
    double  HDOP;       /**< Horizontal dilution of precision */
    double  elv;        /**< Antenna altitude above/below mean sea level (geoid) */
    char    elv_units;  /**< [M]eters (Antenna height unit) */
    double  diff;       /**< Geoidal separation (Diff. between WGS-84 earth ellipsoid and mean sea level. '-' = geoid is below WGS-84 ellipsoid) */
    char    diff_units; /**< [M]eters (Units of geoidal separation) */
    double  dgps_age;   /**< Time in seconds since last DGPS update */
    int32_t     dgps_sid;   /**< DGPS station ID number */

} GPSMNG_MSG_GGA;

/**
 * GSA packet information structure (Satellite status)
 */
typedef struct _nmeaGPGSA
{
    char    fix_mode;   /**< Mode (M = Manual, forced to operate in 2D or 3D; A = Automatic, 3D/2D) */
    int32_t     fix_type;   /**< Type, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D) */
    int32_t     sat_prn[NMEA_MAXSAT]; /**< PRNs of satellites used in position fix (null for unused fields) */
    double  PDOP;       /**< Dilution of precision */
    double  HDOP;       /**< Horizontal dilution of precision */
    double  VDOP;       /**< Vertical dilution of precision */
    int32_t     id;         /*< GNSS system ID number defined by NMEA*/

} GPSMNG_MSG_GSA;

/**
 * Information about satellite
 * @see nmeaSATINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATELLITE
{
    int32_t     id;         /**< Satellite PRN number */
    int32_t     in_use;     /**< Used in position fix */
    int32_t     elv;        /**< Elevation in degrees, 90 maximum */
    int32_t     azimuth;    /**< Azimuth, degrees from true north, 000 to 359 */
    int32_t     sig;        /**< Signal, 00-99 dB */

} nmeaSATELLITE;

/**
 * GSV packet information structure (Satellites in view)
 */
typedef struct _nmeaGPGSV
{
    int32_t     pack_count; /**< Total number of messages of this type in this cycle */
    int32_t     pack_index; /**< Message number */
    int32_t     sat_count;  /**< Total number of satellites in view */
    nmeaSATELLITE sat_data[NMEA_SATINPACK];
    int32_t     id;         /*< GNSS signal ID number defined by NMEA*/
} GPSMNG_MSG_GSV;

/**
 * RMC packet information structure (Recommended Minimum sentence C)
 */
typedef struct _nmeaGPRMC
{
    nmeaTIME utc;       /**< UTC of position */
    char    status;     /**< Status (A = active or V = void) */
	double  lat;        /**< Latitude in NDEG - [degree][min].[sec/60] */
    char    ns;         /**< [N]orth or [S]outh */
	double  lon;        /**< Longitude in NDEG - [degree][min].[sec/60] */
    char    ew;         /**< [E]ast or [W]est */
    double  speed;      /**< Speed over the ground in knots */
    double  direction;  /**< Track angle in degrees True */
    double  declination; /**< Magnetic variation degrees (Easterly var. subtracts from true course) */
    char    declin_ew;  /**< [E]ast or [W]est */
    char    mode;       /**< Mode indicator of fix type (A = autonomous, D = differential, E = estimated, N = not valid, S = simulator) */

} GPSMNG_MSG_RMC;

/**
 * VTG packet information structure (Track made good and ground speed)
 */
typedef struct _nmeaGPVTG
{
    double  dir;        /**< True track made good (degrees) */
    char    dir_t;      /**< Fixed text 'T' indicates that track made good is relative to true north */
    double  dec;        /**< Magnetic track made good */
    char    dec_m;      /**< Fixed text 'M' */
    double  spn;        /**< Ground speed, knots */
    char    spn_n;      /**< Fixed text 'N' indicates that speed over ground is in knots */
    double  spk;        /**< Ground speed, kilometers per hour */
    char    spk_k;      /**< Fixed text 'K' indicates that speed over ground is in kilometers/hour */

} GPSMNG_MSG_VTG;

typedef struct _nmeaGPGLL{
    double  lat;        /**< Latitude in NDEG - [degree][min].[sec/60] */
    char    ns;         /**< [N]orth or [S]outh */
	double  lon;        /**< Longitude in NDEG - [degree][min].[sec/60] */
    char    ew;         /**< [E]ast or [W]est */
    nmeaTIME utc;       /**< UTC of position */
    char    status;     /**< Status (A = active or V = void) */
    char    mode;       /**< Mode indicator of fix type (A = autonomous, D = differential, E = estimated, N = not valid, S = simulator) */
}GPSMNG_MSG_GLL;

typedef struct _nmeaGPGST{
    nmeaTIME utc;       /**< UTC of position */
    double pdsd;        /**< Pseudo distance standard deviation*/
    double sdlh;        /**< Standard deviation of the long half axis of an ellipsoid*/
    double sdsh;        /**< Standard deviation of the short half axis of an ellipsoid */
    double olh;         /**< Orientation of the long half axis of an ellipsoid */
    double slad;        /**< Standard latitude deviation */
    double slod;        /**< Standard Longitude Deviation */
    double shd;         /**< Standard height deviation */
}GPSMNG_MSG_GST;

typedef struct _nmeaGPZDA{
    nmeaTIME utc;       /**< UTC of position */
    int32_t hour;           /*< Local Area Hours*/
    int32_t min;           /*< Local Area mins*/
}GPSMNG_MSG_ZDA;

typedef struct GPSMNG_MSG_PACKET
{
    GPSMNG_MSG_RMC gpsRMC;
    GPSMNG_MSG_GGA gpsGGA;
    GPSMNG_MSG_GLL gpsGLL;
    GPSMNG_MSG_GSA gpsGSA;
    GPSMNG_MSG_VTG gpsVTG;
    GPSMNG_MSG_GSV gpsGSV[GPSMNG_GSV_MAX_MSG_NUM];
    GPSMNG_MSG_GST gpsGST;
    GPSMNG_MSG_ZDA gpsZDA;
} GPSMNG_MSG_PACKET;

/* FAA mode added to some fields in NMEA 2.3. */
typedef enum GPSMNG_FAA_MODE
{
    GPS_FAA_MODE_AUTONOMOUS = 'A',
    GPS_FAA_MODE_DIFFERENTIAL = 'D',
    GPS_FAA_MODE_ESTIMATED = 'E',
    GPS_FAA_MODE_MANUAL = 'M',
    GPS_FAA_MODE_SIMULATED = 'S',
    GPS_FAA_MODE_NOT_VALID = 'N',
    GPS_FAA_MODE_PRECISE = 'P',
} GPSMNG_FAA_MODE;

typedef struct GPSMNG_STATE_INFO
{
    int32_t nr;
    int32_t elevation;
    int32_t azimuth;
    int32_t snr;
} GPSMNG_STATE_INFO;

/** get data callback */
typedef int32_t (*GPSMNG_CALLBACK_FN_PTR)(GPSMNG_MSG_PACKET *msgPacket, void* privateData);

/** get gps data */
typedef struct GPSMNG_CALLBACK
{
    GPSMNG_CALLBACK_FN_PTR fnGpsDataCB;
    void* privateData;
} GPSMNG_CALLBACK;

int32_t GPSMNG_Init(void);
int32_t GPSMNG_DeInit(void);

int32_t GPSMNG_Register(GPSMNG_CALLBACK* fnGpsCB);
int32_t GPSMNG_UnRegister(GPSMNG_CALLBACK* fnGpsCB);

int32_t GPSMNG_Start(void);
int32_t GPSMNG_Stop(void);

int32_t GPSMNG_GetData(GPSMNG_MSG_PACKET* msgPacket);

/* vim: set ts=4 sw=4 et: */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
