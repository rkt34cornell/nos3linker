#include "cfe.h"
#include <string.h>
#include "macsm_mgr_events.h"
#include "macsm_mgr_msg.h"
#include "macsm_mgr_msgdefs.h"
#include "macsm_mgr_msgids.h"
#include "macsm_mgr_perfids.h"

typedef struct
{
    CFE_SB_PipeId_t CmdPipe;
    CFE_ES_AppRunStatus_t RunStatus;
} MACSM_MGR_AppData_t;

static MACSM_MGR_AppData_t MACSM_MGR_AppData;

static const char *MACSM_MGR_Denylist[] = {
    "CFE_ES",
    "CFE_EVS",
    "CFE_SB",
    "CFE_TBL",
    "CFE_TIME",
    "macsm_mgr",
    NULL
};

static bool MACSM_MGR_IsDenied(const char *name)
{
    for (int i = 0; MACSM_MGR_Denylist[i] != NULL; ++i)
    {
        if (strcmp(MACSM_MGR_Denylist[i], name) == 0)
        {
            return true;
        }
    }
    return false;
}

static int32 MACSM_MGR_Unload(const MACSM_MGR_UnloadCmd_t *cmd)
{
    if (MACSM_MGR_IsDenied(cmd->Name))
    {
        CFE_EVS_SendEvent(MACSM_MGR_DENY_EID, CFE_EVS_EventType_ERROR, "MACSM_MGR: Denied unload of %s", cmd->Name);
        return CFE_STATUS_BAD_ARGUMENT;
    }
    CFE_ES_AppId_t id;
    int32 status = CFE_ES_GetAppIDByName(&id, cmd->Name);
    if (status == CFE_SUCCESS)
    {
        status = CFE_ES_DeleteApp(id);
        if (status == CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(MACSM_MGR_UNLOAD_INF_EID, CFE_EVS_EventType_INFORMATION, "Unloaded app %s", cmd->Name);
        }
        else
        {
            CFE_EVS_SendEvent(MACSM_MGR_ERROR_EID, CFE_EVS_EventType_ERROR, "DeleteApp failed for %s: 0x%08X", cmd->Name, (unsigned int)status);
        }
    }
    else
    {
        CFE_EVS_SendEvent(MACSM_MGR_ERROR_EID, CFE_EVS_EventType_ERROR, "GetAppIDByName failed for %s: 0x%08X", cmd->Name, (unsigned int)status);
    }
    return status;
}

static int32 MACSM_MGR_Load(const MACSM_MGR_LoadCmd_t *cmd)
{
    CFE_ES_AppId_t id;
    int32 status = CFE_ES_StartApp(&id, cmd->Name, cmd->Entry, cmd->Path, cmd->StackSize, cmd->Priority);
    if (status == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MACSM_MGR_LOAD_INF_EID, CFE_EVS_EventType_INFORMATION, "Loaded app %s", cmd->Name);
    }
    else
    {
        CFE_EVS_SendEvent(MACSM_MGR_ERROR_EID, CFE_EVS_EventType_ERROR, "StartApp failed for %s: 0x%08X", cmd->Name, (unsigned int)status);
    }
    return status;
}

static int32 MACSM_MGR_Reload(const MACSM_MGR_ReloadCmd_t *cmd)
{
    if (MACSM_MGR_IsDenied(cmd->Name))
    {
        CFE_EVS_SendEvent(MACSM_MGR_DENY_EID, CFE_EVS_EventType_ERROR, "MACSM_MGR: Denied reload of %s", cmd->Name);
        return CFE_STATUS_BAD_ARGUMENT;
    }
    CFE_ES_AppId_t id;
    int32 status = CFE_ES_GetAppIDByName(&id, cmd->Name);
    if (status == CFE_SUCCESS)
    {
        status = CFE_ES_ReloadApp(id, cmd->Path);
        if (status == CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(MACSM_MGR_RELOAD_INF_EID, CFE_EVS_EventType_INFORMATION, "Reloaded app %s", cmd->Name);
        }
        else
        {
            CFE_EVS_SendEvent(MACSM_MGR_ERROR_EID, CFE_EVS_EventType_ERROR, "ReloadApp failed for %s: 0x%08X", cmd->Name, (unsigned int)status);
        }
    }
    else
    {
        CFE_EVS_SendEvent(MACSM_MGR_ERROR_EID, CFE_EVS_EventType_ERROR, "GetAppIDByName failed for %s: 0x%08X", cmd->Name, (unsigned int)status);
    }
    return status;
}

static void MACSM_MGR_ProcessCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t fcn;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &fcn);
    switch (fcn)
    {
        case MACSM_MGR_UNLOAD_CC:
            MACSM_MGR_Unload((const MACSM_MGR_UnloadCmd_t *)SBBufPtr);
            break;
        case MACSM_MGR_LOAD_CC:
            MACSM_MGR_Load((const MACSM_MGR_LoadCmd_t *)SBBufPtr);
            break;
        case MACSM_MGR_RELOAD_CC:
            MACSM_MGR_Reload((const MACSM_MGR_ReloadCmd_t *)SBBufPtr);
            break;
        default:
            CFE_EVS_SendEvent(MACSM_MGR_ERROR_EID, CFE_EVS_EventType_ERROR, "Invalid function code %u", fcn);
            break;
    }
}

static int32 MACSM_MGR_AppInit(void)
{
    MACSM_MGR_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_ES_RegisterApp();
    CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    CFE_SB_CreatePipe(&MACSM_MGR_AppData.CmdPipe, 4, "MACSM_MGR_CMD_PIPE");
    CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MACSM_MGR_CMD_MID), MACSM_MGR_AppData.CmdPipe);
    CFE_EVS_SendEvent(MACSM_MGR_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "MACSM Manager Initialized");
    return CFE_SUCCESS;
}

void MACSM_MGR_AppMain(void)
{
    CFE_ES_PerfLogEntry(MACSM_MGR_MAIN_TASK_PERF_ID);
    int32 status = MACSM_MGR_AppInit();
    if (status != CFE_SUCCESS)
    {
        MACSM_MGR_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&MACSM_MGR_AppData.RunStatus))
    {
        CFE_ES_PerfLogExit(MACSM_MGR_MAIN_TASK_PERF_ID);
        CFE_SB_Buffer_t *SBBufPtr;
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, MACSM_MGR_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        CFE_ES_PerfLogEntry(MACSM_MGR_MAIN_TASK_PERF_ID);
        if (status == CFE_SUCCESS)
        {
            MACSM_MGR_ProcessCommand(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(MACSM_MGR_ERROR_EID, CFE_EVS_EventType_ERROR, "SB receive failed: 0x%08X", (unsigned int)status);
            MACSM_MGR_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(MACSM_MGR_MAIN_TASK_PERF_ID);
    CFE_ES_ExitApp(MACSM_MGR_AppData.RunStatus);
}

