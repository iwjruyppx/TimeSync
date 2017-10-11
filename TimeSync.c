
/* Standard includes. */
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "TimeSync.h"

#define LOGE(pHandle, fmt, args...) \
    if(NULL != pHandle->pConfig->LOG)\
        pHandle->pConfig->LOG(fmt, ##args)
#define TICKS_DIFF64(prev, cur) ((cur) >= (prev)) ? ((cur)-(prev)) : ((0xFFFFFFFFFFFFFFFF-(prev))+1+(cur))

static int64_t captureModule(struct TimeSyncH_t *pHandle, int64_t systemTime, int64_t referenceTime, double slop)
{
    int64_t refOffsetDiff = TICKS_DIFF64(pHandle->time.referenceTime, referenceTime);
    int64_t sysOffsetDiff = (int64_t)((double)TICKS_DIFF64(pHandle->time.systemTime, systemTime) * slop);
    int64_t avgOffsetDiff = (int64_t)((double)refOffsetDiff *0.3f + (double)sysOffsetDiff *0.7f);
    int64_t offsetDiff = TICKS_DIFF64(pHandle->syncSysTime, pHandle->time.referenceTime + avgOffsetDiff);
    return offsetDiff;
}

static int inputTime(struct TimeSyncH_t *pHandle, int64_t systemTime, int64_t referenceTime, double slop)
{
    int64_t offsetDiff = TICKS_DIFF64(pHandle->syncSysTime, referenceTime);
    
    switch(pHandle->time.status)
    {
        case 0:
            /*System initial*/
            pHandle->syncSysTime = referenceTime;
            pHandle->bias = 0;
            LOGE(pHandle, "inputTime init: %d\n", (int)referenceTime);
            break;
        default:
            if(referenceTime - pHandle->time.referenceTime < 100000000ll)
                return 0;
            
            if(llabs(offsetDiff) > pHandle->pConfig->offsetMaxTolerance)
            {
                pHandle->syncSysTime = referenceTime;
                pHandle->bias = 0;
                LOGE(pHandle, "inputTime Fail(offsetMaxTolerance): %d, tolerance %d\n", (int)offsetDiff, (int)pHandle->pConfig->offsetMaxTolerance);
            }else{
                pHandle->bias = captureModule(pHandle, systemTime, referenceTime, slop);
            }
            break;
    }
    
    pHandle->time.referenceTime = referenceTime;
    pHandle->time.systemTime = systemTime;
    pHandle->time.status++;
    return 0;
}

static int incrementTime(struct TimeSyncH_t *pHandle, int64_t systemTime, double slop)
{
    int64_t biasRevised;
    int64_t timeDiff;
    
    /*Initial pre system timestamp*/
    if(pHandle->preSyncTime == 0ll)
        pHandle->preSyncTime = systemTime;

    /*Calculate system timediff */
    timeDiff = (int64_t)((double)(systemTime - pHandle->preSyncTime) * slop);
    pHandle->preSyncTime = systemTime;

    /*Calculate max bias revised*/
    biasRevised = (int64_t)((double)timeDiff * pHandle->pConfig->biasMaxTolerance);

    /*system time calculate*/
    if(pHandle->bias > 0)
    {
        if(pHandle->bias < biasRevised)
        {
            biasRevised = pHandle->bias;
        }
        pHandle->bias -= biasRevised;
        pHandle->syncSysTime += (timeDiff + biasRevised);
    }else{
        if(llabs(pHandle->bias) < biasRevised)
        {
            biasRevised = llabs(pHandle->bias);
        }
        pHandle->bias += biasRevised;
        pHandle->syncSysTime += (timeDiff - biasRevised);
    }
    
    return 0;
}

static int reset(struct TimeSyncH_t *pHandle, int64_t systemTime, int64_t referenceTime)
{

    return 0;
}

static int64_t getTime(struct TimeSyncH_t *pHandle)
{
    return pHandle->syncSysTime;
}

int TimeSyncInit(pTimeSyncHandle_t pHandle, pTimeSyncConfig pConfig)
{
    pHandle->pConfig = pConfig;
    pHandle->inputTime = inputTime;
    pHandle->incrementTime = incrementTime;
    pHandle->reset = reset;
    pHandle->getTime = getTime;
    pHandle->time.status = 0;
    pHandle->syncSysTime = 0ll;
    pHandle->bias = 0ll;
    pHandle->preSyncTime = 0ll;
    return 0;
}