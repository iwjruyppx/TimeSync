
#ifndef __TimeSync_H
#define __TimeSync_H

#define RELIABLE 0
#define UNRELIABLE -1

#define MAX_TIMESYNC_SAMPLE         20
typedef struct {
    int status;
    int64_t systemTime;
    int64_t referenceTime;
}StoreTimeInfo_t, *pStoreTimeInfo_t;

typedef struct{
    /*Slop tolerance verify, if set tolerance = 0.1f, that mean slop> 1.1 && slop < 0.9 will return fail and ignore this sync.*/
    float slopMaxTolerance;
    /*Minimun sync timestamp period(ns)*/
    int64_t minSyncTime;
    /*Max system time and reference time difference tolerance*/
    int64_t offsetMaxTolerance;
    /*Max bias tolerance, %of timestamp increment diff*/
    double  biasMaxTolerance;
    
    /*if you want to know error info, we will call this function to report log or error message*/
    int (*LOG)(const char * ,...);
    /*Return current system timestamp*/
    int64_t (*getTimestamp)(void);
    /*Avarge slop update */
    int (*avargeSlopCallback)(double);
} TimeSyncConfig, *pTimeSyncConfig;


/************************************************************/
/********** Slop *******************/
/************************************************************/
typedef struct TimeSlopH_t{
    /*inputRefTime(handle, system timestamp ns, reference timestamp ns)*/
    int (*inputTime)(struct TimeSlopH_t *, int64_t, int64_t);
    /*inputRefTime(handle, reference timestamp ns, sync timestamp ns)*/
    double (*getSlop)(struct TimeSlopH_t *);
    int (*reset)(struct TimeSlopH_t *);
    
    pTimeSyncConfig pConfig;
    int front;
    int rear;
    StoreTimeInfo_t data[MAX_TIMESYNC_SAMPLE];
    double avargeSlop;
} TimeSlopHandle_t, *pTimeSlopHandle_t;
/************************************************************/
int TimeSlopInit(pTimeSlopHandle_t pHandle, pTimeSyncConfig pConfig);
/************************************************************/

/************************************************************/
/********** TimeSync *******************/
/************************************************************/
typedef struct TimeSyncH_t{
    /*inputRefTime(handle, reference timestamp ns)*/
    int (*inputTime)(struct TimeSyncH_t *, int64_t);
    /*incrementTime(handle, system timestamp ns, slop)*/
    int (*incrementTime)(struct TimeSyncH_t *, int64_t, double);
    /*inputRefTime(handle, system timestamp ns, reference timestamp ns)*/
    int (*reset)(struct TimeSyncH_t *, int64_t, int64_t);
    int64_t (*getTime)(struct TimeSyncH_t *);
    
    pTimeSyncConfig pConfig;
    StoreTimeInfo_t time;
    int64_t bias;
    int64_t preSyncTime;
} TimeSyncHandle_t, *pTimeSyncHandle_t;
/************************************************************/
int TimeSyncInit(pTimeSyncHandle_t pHandle, pTimeSyncConfig pConfig);
/************************************************************/


#endif /* __TimeSync_H */
