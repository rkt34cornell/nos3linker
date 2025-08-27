#ifndef MACSM_MGR_MSG_H
#define MACSM_MGR_MSG_H

#include "cfe.h"

#define MACSM_MGR_MAX_NAME_LEN OS_MAX_API_NAME
#define MACSM_MGR_MAX_PATH_LEN OS_MAX_PATH_LEN

typedef struct
{
    char Name[MACSM_MGR_MAX_NAME_LEN];
} MACSM_MGR_UnloadCmd_t;

typedef struct
{
    char Name[MACSM_MGR_MAX_NAME_LEN];
    char Entry[MACSM_MGR_MAX_NAME_LEN];
    char Path[MACSM_MGR_MAX_PATH_LEN];
    uint32 StackSize;
    uint32 Priority;
} MACSM_MGR_LoadCmd_t;

typedef struct
{
    char Name[MACSM_MGR_MAX_NAME_LEN];
    char Path[MACSM_MGR_MAX_PATH_LEN];
} MACSM_MGR_ReloadCmd_t;

#endif
