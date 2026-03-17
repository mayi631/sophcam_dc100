#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "liveviewmng.h"
#ifdef SERVICES_LIVEVIEW_ON
#include "liveview.h"
#endif
#include "cmdmng.h"

#ifndef SERVICES_LIVEVIEW_ON
#define CMD_CLIENT_ID_LIVEVIEW (0)
typedef enum cmd_liveview_e {
    CMD_LIVEVIEW_INVALID = 0,
    CMD_LIVEVIEW_SHUTDOWN,
    CMD_LIVEVIEW_SWITCH,
    CMD_LIVEVIEW_MOVEUP,
    CMD_LIVEVIEW_MOVEDOWN,
    CMD_LIVEVIEW_MIRROR,
    CMD_LIVEVIEW_FILP,
    CMD_LIVEVIEW_ADJUSTFOCUS,
    CMD_LIVEVIEW_MAX
} cmd_liveview_t;
#define CMD_CHANNEL_ID_LIVEVIEW(liveview_id) (0x00 + (liveview_id))
#endif


int32_t LIVEVIEWMNG_Switch(uint32_t val)
{
    CMDMNG_SendMqCmd(CMD_CLIENT_ID_LIVEVIEW, CMD_CHANNEL_ID_LIVEVIEW(0), CMD_LIVEVIEW_SWITCH, val);
    return 0;
}

int32_t LIVEVIEWMNG_AdjustFocus(int32_t wndid , char* ratio)
{
    //todo need ack
    CMDMNG_SendMqCmd_Str(CMD_CLIENT_ID_LIVEVIEW, CMD_CHANNEL_ID_LIVEVIEW(0), CMD_LIVEVIEW_ADJUSTFOCUS, wndid , ratio);
    return 0;
}

int32_t LIVEVIEWMNG_MoveUp(int32_t wndid)
{
    //todo need ack
    CMDMNG_SendMqCmd(CMD_CLIENT_ID_LIVEVIEW, CMD_CHANNEL_ID_LIVEVIEW(0), CMD_LIVEVIEW_MOVEUP, wndid);
    return 0;
}

int32_t LIVEVIEWMNG_MoveDown(int32_t wndid)
{
    //todo need ack
    CMDMNG_SendMqCmd(CMD_CLIENT_ID_LIVEVIEW, CMD_CHANNEL_ID_LIVEVIEW(0), CMD_LIVEVIEW_MOVEDOWN, wndid);
    return 0;
}

int32_t LIVEVIEWMNG_Mirror(uint32_t val)
{
    //todo need ack
    CMDMNG_SendMqCmd(CMD_CLIENT_ID_LIVEVIEW, CMD_CHANNEL_ID_LIVEVIEW(0), CMD_LIVEVIEW_MIRROR, val);
    return 0;
}

int32_t LIVEVIEWMNG_Filp(uint32_t val)
{
    //todo need ack
    CMDMNG_SendMqCmd(CMD_CLIENT_ID_LIVEVIEW, CMD_CHANNEL_ID_LIVEVIEW(0), CMD_LIVEVIEW_FILP, val);
    return 0;
}
