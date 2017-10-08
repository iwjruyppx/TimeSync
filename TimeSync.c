
/* Standard includes. */
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "TimeSync.h"

#define GET_EMPTY_SIZE(len, front, real) ((front) >=(real)) ? (len-(front-real)) : (real-front)
#define GET_USED_SIZE(len, front, real) ((front) >=(real)) ? (front-real) : (len-(real-front))
#define CHECK_OVERWRITE(len, front, real) (((front+1)%len) == real) ? 1 : 0
#define CALCULATE_SLOP(target, local) (double)target/(double)local
#define LOGE(pHandle, fmt, args...) \
    if(NULL != pHandle->pConfig->LOG)\
        pHandle->pConfig->LOG(fmt, ##args)

static int slopVerify(struct TimeSlopH_t * pHandle, double slop)
{
    if(slop > (1.0f + pHandle->pConfig->slopMaxTolerance))
    {
        LOGE(pHandle, "slopVerify Fail: %f, tolerance %f\n", (float)slop, (float)pHandle->pConfig->slopMaxTolerance);
        return -1;
    }
    if(slop < (1.0f - pHandle->pConfig->slopMaxTolerance))
    {
        LOGE(pHandle, "slopVerify Fail: %f, tolerance %f\n", (float)slop, (float)pHandle->pConfig->slopMaxTolerance);
        return -2;
    }
    return 0;
}

static int syncTimeVerify(struct TimeSlopH_t * pHandle, int64_t preTime, int64_t nowTime)
{
    if((nowTime - preTime) >  pHandle->pConfig->minSyncTime)
    {
        LOGE(pHandle, "syncTimeVerify Fail: %d, tolerance %d\n", (int)(nowTime - preTime), (int)pHandle->pConfig->minSyncTime);
        return -1;
    }
    return 0;
}

static int setTimeToQueue(struct TimeSlopH_t * pHandle, int64_t systemTime, int64_t referenceTime)
{
    pStoreTimeInfo_t pdata = &pHandle->data[pHandle->front];
    pdata->systemTime = systemTime;
    pdata->referenceTime = referenceTime;
    
    if(CHECK_OVERWRITE(MAX_TIMESYNC_SAMPLE, pHandle->front, pHandle->rear))
        pHandle->rear = (pHandle->rear+1)%MAX_TIMESYNC_SAMPLE;
    
    return 0;
}

static pStoreTimeInfo_t getTimeFromQueue(struct TimeSlopH_t * pHandle, int index)
{
    return &pHandle->data[(pHandle->rear + index)%MAX_TIMESYNC_SAMPLE];
}

static double getAvageSlop(struct TimeSlopH_t * pHandle)
{
    pStoreTimeInfo_t pPredata, pdata;
    double tempSlop;
    double avargeSlop = 0.0f;
    int useSize = GET_USED_SIZE(MAX_TIMESYNC_SAMPLE, pHandle->front, pHandle->rear);
    int i;
    if(useSize<2)
        return 1.0f;

    for(i=1;i<(useSize);i++)
    {
        pPredata = getTimeFromQueue(pHandle, i-1);
        pdata = getTimeFromQueue(pHandle, i);
        tempSlop =  CALCULATE_SLOP((pdata->referenceTime-pPredata->referenceTime), (pdata->systemTime-pPredata->systemTime));
        avargeSlop+=tempSlop;
    }
    return avargeSlop = avargeSlop/(useSize-1);
}

static int resetTimeQueue(struct TimeSlopH_t * pHandle)
{
    int i;
    pHandle->front = 0;
    pHandle->rear= 0;
    for(i=0;i<MAX_TIMESYNC_SAMPLE;i++)
    {
        pHandle->data[i].systemTime = 0ll;
        pHandle->data[i].referenceTime= 0ll;
    }
    return 0;
}

/*inputRefTime(handle, system timestamp ns, reference timestamp ns)*/
static int inputTime(struct TimeSlopH_t * pHandle, int64_t systemTime, int64_t referenceTime)
{
    pStoreTimeInfo_t pPredata = NULL;
    double tempSlop;
    int useSize = GET_USED_SIZE(MAX_TIMESYNC_SAMPLE, pHandle->front, pHandle->rear);
    
    switch (useSize)
    {
        case 0:
            setTimeToQueue(pHandle, systemTime, referenceTime);
            break;
        case 1:
            pPredata = getTimeFromQueue(pHandle, (useSize-1));
            if(syncTimeVerify(pHandle, pPredata->referenceTime, referenceTime))
                break;
            
            tempSlop =  CALCULATE_SLOP((referenceTime-pPredata->referenceTime), (systemTime-pPredata->systemTime));
            /*if slop verify fail, maybe initial timestamp something wrong, so reset initial time*/
            if(slopVerify(pHandle, tempSlop))
                resetTimeQueue(pHandle);
            
            setTimeToQueue(pHandle, systemTime, referenceTime);
            break;
        default:
            pPredata = getTimeFromQueue(pHandle, (useSize-1));
            if(syncTimeVerify(pHandle, pPredata->referenceTime, referenceTime))
                break;
            
            tempSlop =  CALCULATE_SLOP((referenceTime-pPredata->referenceTime), (systemTime-pPredata->systemTime));
            /*if slop verify fail, maybe initial timestamp something wrong, so reset initial time*/
            if(slopVerify(pHandle, tempSlop))
                break;
            
            setTimeToQueue(pHandle, systemTime, referenceTime);
            pHandle->avargeSlop = getAvageSlop(pHandle);
            break;

    }
    return 0;
}

static float getSlop(struct TimeSlopH_t * pHandle)
{

    return (float)pHandle->avargeSlop;
}

static int reset(struct TimeSlopH_t * pHandle)
{

    return 0;
}

int tmeSlopInit(pTimeSlopHandle_t pHandle, pTimeSyncConfig pConfig)
{
    resetTimeQueue(pHandle);

    pHandle->inputTime = inputTime;
    pHandle->getSlop = getSlop;
    pHandle->reset = reset;
    return 0;
}

