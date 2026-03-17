/**
* @file    gps_data_analysis.h
* @brief   product gps interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    team
* @date      2021/6/19
* @version

*/
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include "gpsmng.h"
#include "gpsmng_analysis.h"
#include "appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define GPS_MNG_DEBUG 1

#if !defined(GPS_MNG_DEBUG)
#   include <assert.h>
#   define NMEA_ASSERT(x)   assert(x)
#else
#   define NMEA_ASSERT(x)
#endif

#define NMEA_TOKS_COMPARE   (1)
#define NMEA_TOKS_PERCENT   (2)
#define NMEA_TOKS_WIDTH     (3)
#define NMEA_TOKS_TYPE      (4)

#define NMEA_TIMEPARSE_BUF  (256)
#define NMEA_CONVSTR_BUF    (256)

/**
 * \brief Calculate control sum of binary buffer
 */
static int32_t nmea_calc_crc(const char *buff, int32_t buff_sz)
{
    int32_t chsum = 0, it;

    for(it = 0; it < buff_sz; ++it)
        chsum ^= (int32_t)buff[it];
    return chsum;
}

/**
 * \brief Convert string to number
 */
static int32_t nmea_atoi(const char *str, int32_t str_sz, int32_t radix)
{
    char *tmp_ptr;
    char buff[NMEA_CONVSTR_BUF];
    int32_t res = 0;

    if(str_sz < NMEA_CONVSTR_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtol(&buff[0], &tmp_ptr, radix);
    }

    return res;
}

/**
 * \brief Convert string to fraction number
 */
static double nmea_atof(const char *str, int32_t str_sz)
{
    char *tmp_ptr;
    char buff[NMEA_CONVSTR_BUF];
    double res = 0;

    if(str_sz < NMEA_CONVSTR_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtod(&buff[0], &tmp_ptr);
    }

    return res;
}


/**
 * \brief Analyse string (specificate for NMEA sentences)
 */
static int32_t nmea_scanf(const char *buff, int32_t buff_sz, const char *format, ...)
{
    const char *beg_tok;
    const char *end_buf = buff + buff_sz;

    va_list arg_ptr;
    int32_t tok_type = NMEA_TOKS_COMPARE;
    int32_t width = 0;
    const char *beg_fmt = 0;
    int32_t snum = 0, unum = 0;

    int32_t tok_count = 0;
    void *parg_target;

    va_start(arg_ptr, format);

    for(; *format && buff < end_buf; ++format)
    {
        switch(tok_type)
        {
        case NMEA_TOKS_COMPARE:
            if('%' == *format)
                tok_type = NMEA_TOKS_PERCENT;
            else if(*buff++ != *format)
                goto fail;
            break;
        case NMEA_TOKS_PERCENT:
            width = 0;
            beg_fmt = format;
            tok_type = NMEA_TOKS_WIDTH;
        case NMEA_TOKS_WIDTH:
            if(isdigit(*format))
                break;
            {
                tok_type = NMEA_TOKS_TYPE;
                if(format > beg_fmt)
                    width = nmea_atoi(beg_fmt, (int32_t)(format - beg_fmt), 10);
            }
        case NMEA_TOKS_TYPE:
            beg_tok = buff;

            if(!width && ('c' == *format || 'C' == *format) && *buff != format[1])
                width = 1;

            if(width)
            {
                if(buff + width <= end_buf)
                    buff += width;
                else
                    goto fail;
            }
            else
            {
                if(!format[1] || (0 == (buff = (char *)memchr(buff, format[1], end_buf - buff))))
                    buff = end_buf;
            }

            if(buff > end_buf)
                goto fail;

            tok_type = NMEA_TOKS_COMPARE;
            tok_count++;

            parg_target = 0; width = (int32_t)(buff - beg_tok);

            switch(*format)
            {
            case 'c':
            case 'C':
                parg_target = (void *)va_arg(arg_ptr, char *);
                if(width && 0 != (parg_target))
                    *((char *)parg_target) = *beg_tok;
                break;
            case 's':
            case 'S':
                parg_target = (void *)va_arg(arg_ptr, char *);
                if(width && 0 != (parg_target))
                {
                    memcpy(parg_target, beg_tok, width);
                    ((char *)parg_target)[width] = '\0';
                }
                break;
            case 'f':
            case 'g':
            case 'G':
            case 'e':
            case 'E':
                parg_target = (void *)va_arg(arg_ptr, double *);
                if(width && 0 != (parg_target))
                    *((double *)parg_target) = nmea_atof(beg_tok, width);
                break;
            };

            if(parg_target)
                break;
            if(0 == (parg_target = (void *)va_arg(arg_ptr, int32_t *)))
                break;
            if(!width)
                break;

            switch(*format)
            {
            case 'd':
            case 'i':
                snum = nmea_atoi(beg_tok, width, 10);
                memcpy(parg_target, &snum, sizeof(int32_t));
                break;
            case 'u':
                unum = nmea_atoi(beg_tok, width, 10);
                memcpy(parg_target, &unum, sizeof(uint32_t));
                break;
            case 'x':
            case 'X':
                unum = nmea_atoi(beg_tok, width, 16);
                memcpy(parg_target, &unum, sizeof(uint32_t));
                break;
            case 'o':
                unum = nmea_atoi(beg_tok, width, 8);
                memcpy(parg_target, &unum, sizeof(uint32_t));
                break;
            default:
                goto fail;
            };

            break;
        };
    }

fail:
    va_end(arg_ptr);
    return tok_count;
}

static int32_t _nmea_parse_time(const char *buff, int32_t buff_sz, nmeaTIME *res)
{
    int32_t success = 0;

    switch(buff_sz)
    {
    case sizeof("hhmmss") - 1:
        success = (3 == nmea_scanf(buff, buff_sz,
            "%2d%2d%2d", &(res->hour), &(res->min), &(res->sec)
            ));
        break;
    case sizeof("hhmmss.s") - 1:
    case sizeof("hhmmss.ss") - 1:
    case sizeof("hhmmss.sss") - 1:
        success = (4 == nmea_scanf(buff, buff_sz,
            "%2d%2d%2d.%d", &(res->hour), &(res->min), &(res->sec), &(res->hsec)
            ));
        break;
    default:
        CVI_LOGE("Parse of time error (format error)!");
        success = 0;
        break;
    }

    return (success?0:-1);
}

/**
 * \brief Parse RMC packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
static int32_t nmea_parse_RMC(const char *buff, int32_t buff_sz, GPSMNG_MSG_RMC *pack)
{
    // $GNRMC,(1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12)*hh(CR)(LF)
    int32_t nsen;
    char time_buff[NMEA_TIMEPARSE_BUF];
    NMEA_ASSERT(buff && pack);
    memset(pack, 0, sizeof(GPSMNG_MSG_RMC));

#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GNRMC,095554.000,A,2318.1327,N,11319.7252,E,080.0,005.7,081215,001.1,E,A*73";
    buff_sz = strlen(buff);
#endif

    nsen = nmea_scanf(buff + strlen("$GPRMC,"), buff_sz - strlen("$GPRMC,"),
        "%s,%C,%f,%C,%f,%C,%f,%f,%2d%2d%2d,%f,%C,%C*",
        &(time_buff[0]),
        &(pack->status), &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(pack->speed), &(pack->direction),
        &(pack->utc.day), &(pack->utc.mon), &(pack->utc.year),
        &(pack->declination), &(pack->declin_ew), &(pack->mode));

    if(nsen != 12 && nsen != 13 && nsen != 14)
    {
        CVI_LOGE("GPRMC parse error!");
        return false;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int32_t)strlen(&time_buff[0]), &(pack->utc)))
    {
        CVI_LOGE("GPRMC time parse error!");
        return false;
    }

    if(pack->utc.year < 90)
        pack->utc.year += 100;
    pack->utc.mon -= 1;

#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%02d%02d%02d,%C,%f,%C,%f,%C,%f,%f,%02d%02d%02d,%f,%C,%C",
    //     pack->utc.hour, pack->utc.min, pack->utc.sec,
    //     pack->status, pack->lat, pack->ns, pack->lon, pack->ew,
    //     pack->speed, pack->direction,
    //     pack->utc.day, pack->utc.mon, pack->utc.year,
    //     pack->declination, pack->declin_ew, pack->mode);
#endif

    return 1;
}

/**
 * \brief Parse GGA packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
static int32_t nmea_parse_GGA(const char *buff, int32_t buff_sz, GPSMNG_MSG_GGA *pack)
{
    // $GNGGA,(1),(2),(3),(4),(5),(6),(7),(8),(9),M,(10),M,(11),(12)*hh(CR)(LF)
    int32_t nsen;
    char time_buff[NMEA_TIMEPARSE_BUF];
    NMEA_ASSERT(buff && pack);
    memset(pack, 0, sizeof(GPSMNG_MSG_GGA));
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GNGGA,095528.000,2318.1133,N,11319.7210,E,1,06,3.7,55.1,M,-5.4,M,,0000*69";
    buff_sz = strlen(buff);
#endif
    nsen = nmea_scanf(buff + strlen("$GPGGA,"), buff_sz - strlen("$GPGGA,"),
        "%s,%f,%C,%f,%C,%d,%d,%f,%f,%C,%f,%C,%f,%d*",
        &(time_buff[0]),
        &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(pack->sig), &(pack->satinuse), &(pack->HDOP), &(pack->elv), &(pack->elv_units),
        &(pack->diff), &(pack->diff_units), &(pack->dgps_age), &(pack->dgps_sid));
    if(14 != nsen && 13 != nsen)
    {
        CVI_LOGE("GPGGA parse error!");
        return false;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int32_t)strlen(&time_buff[0]), &(pack->utc)))
    {
        CVI_LOGE("GPGGA time parse error!");
        return false;
    }

#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%02d%02d%02d,%f,%C,%f,%C,%d,%d,%f,%f,%C,%f,%C,%f,%d",
    //     pack->utc.hour, pack->utc.min, pack->utc.sec,
    //     pack->lat, pack->ns, pack->lon, pack->ew,
    //     pack->sig, pack->satinuse, pack->HDOP, pack->elv, pack->elv_units,
    //     pack->diff, pack->diff_units, pack->dgps_age, pack->dgps_sid);
#endif

    return 1;
}

/**
 * \brief Parse GSA packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
static int32_t nmea_parse_GSA(const char *buff, int32_t buff_sz, GPSMNG_MSG_GSA *pack)
{
    // $GNGSA,(1),(2),(3),(4),(5),(6),(7)*hh(CR)(LF)
    int32_t nsen;
    NMEA_ASSERT(buff && pack);
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GNGSA,A,3,14,22,24,12,,,,,,,,,4.2,3.7,2.1,1*2D";
    buff_sz = strlen(buff);
#endif
    memset(pack, 0, sizeof(GPSMNG_MSG_GSA));
    nsen = nmea_scanf(buff + strlen("$GPGSA,"), buff_sz - strlen("$GPGSA,"),
        "%C,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%d*",
        &(pack->fix_mode), &(pack->fix_type),
        &(pack->sat_prn[0]), &(pack->sat_prn[1]), &(pack->sat_prn[2]), &(pack->sat_prn[3]), &(pack->sat_prn[4]), &(pack->sat_prn[5]),
        &(pack->sat_prn[6]), &(pack->sat_prn[7]), &(pack->sat_prn[8]), &(pack->sat_prn[9]), &(pack->sat_prn[10]), &(pack->sat_prn[11]),
        &(pack->PDOP), &(pack->HDOP), &(pack->VDOP), &(pack->id));
    if(nsen < 7)
    {
        CVI_LOGE("GPGSA parse error!");
        return false;
    }
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%C,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%d",
    //     pack->fix_mode, pack->fix_type,
    //     pack->sat_prn[0], pack->sat_prn[1], pack->sat_prn[2], pack->sat_prn[3], pack->sat_prn[4], pack->sat_prn[5],
    //     pack->sat_prn[6], pack->sat_prn[7], pack->sat_prn[8], pack->sat_prn[9], pack->sat_prn[10], pack->sat_prn[11],
    //     pack->PDOP, pack->HDOP, pack->VDOP, pack->id);
#endif

    return true;
}

static int32_t nmea_parse_GLL(const char *buff, int32_t buff_sz, GPSMNG_MSG_GLL *pack)
{
    // $GNGLL,(1),(2),(3),(4),(5),(6),(7)*hh(CR)(LF)
    char time_buff[NMEA_TIMEPARSE_BUF];
    NMEA_ASSERT(buff && pack);
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*41";
    buff_sz = strlen(buff);
#endif
    memset(pack, 0, sizeof(GPSMNG_MSG_GLL));

   if(7 != nmea_scanf(buff + strlen("$GPGLL,"), buff_sz - strlen("$GPGLL,"),
        "%f,%C,%f,%C,%s,%C,%C*",
        &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(time_buff[0]), &(pack->status), &(pack->mode)))
    {
        CVI_LOGE("GPGLL parse error!");
        return false;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int32_t)strlen(&time_buff[0]), &(pack->utc)))
    {
        CVI_LOGE("GPGLL time parse error!");
        return false;
    }
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%f,%C,%f,%C,%02d%02d%02d,%C,%C",
    //     pack->lat, pack->ns, pack->lon, pack->ew,
    //     pack->utc.hour, pack->utc.min, pack->utc.sec,
    //     pack->status, pack->mode);
#endif
    return true;
}

static int32_t nmea_parse_GST(const char *buff, int32_t buff_sz, GPSMNG_MSG_GST *pack)
{
    // $GPGST,(1),(2),(3),(4),(5),(6),(7),(8)*hh(CR)(LF)
    char time_buff[NMEA_TIMEPARSE_BUF];
    NMEA_ASSERT(buff && pack);
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GPGST,024603.00,3.2,6.6,4.7,47.3,5.8,5.6,22.0*58";
    buff_sz = strlen(buff);
#endif
    memset(pack, 0, sizeof(GPSMNG_MSG_GST));

    if(8 != nmea_scanf(buff + strlen("$GPGST,"), buff_sz - strlen("$GPGST,"),
        "%s,%f,%f,%f,%f,%f,%f,%f*",
        &(time_buff[0]), &(pack->pdsd), &(pack->sdlh), &(pack->sdsh),
        &(pack->olh), &(pack->slad), &(pack->slod), &(pack->shd)))
    {
        CVI_LOGE("GPGST parse error!");
        return false;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int32_t)strlen(&time_buff[0]), &(pack->utc)))
    {
        CVI_LOGE("GPGST time parse error!");
        return false;
    }
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%02d%02d%02d,%f,%f,%f,%f,%f,%f,%f",
    //     pack->utc.hour, pack->utc.min, pack->utc.sec,pack->pdsd, pack->sdlh, pack->sdsh,
    //     pack->olh, pack->slad, pack->slod, pack->shd);
#endif
    return true;
}

/**
 * \brief Parse GSV packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
static int32_t nmea_parse_GSV(const char *buff, int32_t buff_sz, GPSMNG_MSG_GSV *pack)
{
    // $GPGSV,(1),(2),(3),...,(4),(5),(6),(7),(8)*hh(CR)(LF)
    int32_t nsen, nsat;
    NMEA_ASSERT(buff && pack);
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GPGSV,3,1,11,18,73,129,19,10,71,335,40,22,63,323,41,25,49,127,06,0*78";
    buff_sz = strlen(buff);
#endif
    memset(pack, 0, sizeof(GPSMNG_MSG_GSV));

    nsen = nmea_scanf(buff + strlen("$GPGSV,"), buff_sz - strlen("$GPGSV,"),
        "%d,%d,%d,"
        "%d,%d,%d,%d,"
        "%d,%d,%d,%d,"
        "%d,%d,%d,%d,"
        "%d,%d,%d,%d*",
        &(pack->pack_count), &(pack->pack_index), &(pack->sat_count),
        &(pack->sat_data[0].id), &(pack->sat_data[0].elv), &(pack->sat_data[0].azimuth), &(pack->sat_data[0].sig),
        &(pack->sat_data[1].id), &(pack->sat_data[1].elv), &(pack->sat_data[1].azimuth), &(pack->sat_data[1].sig),
        &(pack->sat_data[2].id), &(pack->sat_data[2].elv), &(pack->sat_data[2].azimuth), &(pack->sat_data[2].sig),
        &(pack->sat_data[3].id), &(pack->sat_data[3].elv), &(pack->sat_data[3].azimuth), &(pack->sat_data[3].sig),
        &(pack->id));

    nsat = (pack->pack_index - 1) * NMEA_SATINPACK;
    nsat = (nsat + NMEA_SATINPACK > pack->sat_count)?pack->sat_count - nsat:NMEA_SATINPACK;
    nsat = nsat * 4 + 4 /* first three sentence`s */;

    if(nsen < nsat || nsen > (NMEA_SATINPACK * 4 + 4))
    {
        // CVI_LOGE("GPGSV parse error!");
        return false;
    }

    return 1;
}

/**
 * \brief Parse VTG packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
static int32_t nmea_parse_VTG(const char *buff, int32_t buff_sz, GPSMNG_MSG_VTG *pack)
{
    // $GNVTG,(1),T,(2),M,(3),N,(4),K,(5)*hh(CR)(LF)
    NMEA_ASSERT(buff && pack);
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GNVTG,005.7,T,008.0,M,080.0,N,010.0,K,A*11";
    buff_sz = strlen(buff);
#endif
    memset(pack, 0, sizeof(GPSMNG_MSG_VTG));

    if(8 != nmea_scanf(buff + strlen("$GPVTG,"), buff_sz - strlen("$GPVTG,"),
        "%f,%C,%f,%C,%f,%C,%f,%C*",
        &(pack->dir), &(pack->dir_t),
        &(pack->dec), &(pack->dec_m),
        &(pack->spn), &(pack->spn_n),
        &(pack->spk), &(pack->spk_k)))
    {
        CVI_LOGE("GPVTG parse error!");
        return false;
    }

    if( pack->dir_t != 'T' ||
        pack->dec_m != 'M' ||
        pack->spn_n != 'N' ||
        pack->spk_k != 'K')
    {
        CVI_LOGE("GPVTG parse error (format error)!");
        return false;
    }
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%f,%C,%f,%C,%f,%C,%f,%C",
    //     pack->dir, pack->dir_t,pack->dec, pack->dec_m,
    //     pack->spn, pack->spn_n,pack->spk, pack->spk_k);
#endif
    return 1;
}

static int32_t nmea_parse_ZDA(const char *buff, int32_t buff_sz, GPSMNG_MSG_ZDA *pack)
{
    // $GNZDA,(1),(2),(3),(4),(5),(6)*hh(CR)(LF)
    NMEA_ASSERT(buff && pack);
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%s", buff);
    buff = (const char *)"$GNZDA,095555.000,08,12,2015,00,00*4C";
    buff_sz = strlen(buff);
#endif
    char time_buff[NMEA_TIMEPARSE_BUF];
    memset(pack, 0, sizeof(GPSMNG_MSG_ZDA));

    if(6 != nmea_scanf(buff + strlen("$GPZDA,"), buff_sz - strlen("$GPZDA,"),
        "%s,%02d,%02d,%04d,%02d,%02d*",
        &(time_buff[0]), &(pack->utc.day), &(pack->utc.mon), &(pack->utc.year), &(pack->hour), &(pack->min)))
    {
        CVI_LOGE("GPZDA parse error!");
        return false;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int32_t)strlen(&time_buff[0]), &(pack->utc)))
    {
        CVI_LOGE("GPZDA time parse error!");
        return false;
    }
#ifdef GPS_MNG_DEBUG
    // CVI_LOGD("%02d%02d%02d,%02d,%02d,%04d,%02d,%02d",
    //     pack->utc.hour, pack->utc.min, pack->utc.sec,
    //     pack->utc.day, pack->utc.mon, pack->utc.year, pack->hour, pack->min);
#endif
    return true;
}

int32_t nmea_parse_rawdata(GPSMNG_RAW_DATA* gpsRawData, GPSMNG_MSG_PACKET* gpsMsgPack)
{
    int32_t s32Ret = 0;

    /* parser RMC rawdata */
    s32Ret = nmea_parse_RMC(gpsRawData->rmcStr, strlen(gpsRawData->rmcStr), &(gpsMsgPack->gpsRMC));
    if (s32Ret != true)
    {
        CVI_LOGD("parser RMC str failed!\n");
    }

    /* parser GGA rawdata*/
    s32Ret = nmea_parse_GGA(gpsRawData->ggaStr, strlen(gpsRawData->ggaStr), &(gpsMsgPack->gpsGGA));
    if (s32Ret != true)
    {
        CVI_LOGD("parser GGA str failed!\n");
    }

    /* parser GLL rawdata*/
    s32Ret = nmea_parse_GLL(gpsRawData->gllStr, strlen(gpsRawData->gllStr), &(gpsMsgPack->gpsGLL));
    if (s32Ret != true)
    {
        CVI_LOGD("parser GLL str failed!\n");
    }

    /* parser GSA rawdata*/
    s32Ret = nmea_parse_GSA(gpsRawData->gsaStr, strlen(gpsRawData->gsaStr), &(gpsMsgPack->gpsGSA));
    if (s32Ret != true)
    {
        CVI_LOGD("parser GSA str failed!\n");
    }

    /* parser VTG rawdata*/
    s32Ret = nmea_parse_VTG(gpsRawData->vtgStr, strlen(gpsRawData->gsaStr), &(gpsMsgPack->gpsVTG));
    if (s32Ret != true)
    {
        CVI_LOGD("parser VTG str failed!\n");
    }

    /* parser GSV rawdata*/
    for (int32_t i = 0; i < GPSMNG_GSV_MAX_MSG_NUM ; i++)
    {
        s32Ret = nmea_parse_GSV(gpsRawData->gsvStr[i], strlen(gpsRawData->gsvStr[i]), &(gpsMsgPack->gpsGSV[i]));
        if (s32Ret != true)
        {
            // CVI_LOGD("parser GSV str [%d/%d] failed!\n", i, GPSMNG_GSV_MAX_MSG_NUM);
        }
    }

    return false;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
