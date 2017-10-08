
#ifndef __TimeSync_H
#define __TimeSync_H

#define MAX_TIMESYNC_SAMPLE         20

typedef struct {
    int64_t systemTime;
    int64_t referenceTime;
}StoreTimeInfo_t, *pStoreTimeInfo_t;


typedef struct{
    /*Slop tolerance verify, if set tolerance = 0.1f, that mean slop> 1.1 && slop < 0.9 will return fail and ignore this sync.*/
    float slopMaxTolerance;
    /*Minimun sync timestamp period(ns)*/
    int64_t minSyncTime;
    
    /*if you want to know error info, we will call this function to report log or error message*/
    int (*LOG)(const char * ,...);
    /*Return current system timestamp*/
    int64_t (*getTimestamp)(void);
} TimeSyncConfig, *pTimeSyncConfig;

typedef struct TimeSlopH_t{
    /*inputRefTime(handle, system timestamp ns, reference timestamp ns)*/
    int (*inputTime)(struct TimeSlopH_t *, int64_t, int64_t);
    /*inputRefTime(handle, reference timestamp ns, sync timestamp ns)*/
    float (*getSlop)(struct TimeSlopH_t *);
    int (*reset)(struct TimeSlopH_t *);
    
    pTimeSyncConfig pConfig;
    int front;
    int rear;
    StoreTimeInfo_t data[MAX_TIMESYNC_SAMPLE];
    double avargeSlop;
} TimeSlopHandle_t, *pTimeSlopHandle_t;

int timeSlopInit(pTimeSlopHandle_t pHandle, pTimeSyncConfig pConfig);

#endif /* __TimeSync_H */
