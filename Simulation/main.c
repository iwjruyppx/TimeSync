#include "main.h"
#include "TimeSync.h"

TimeSlopHandle_t SlopHandle;
TimeSyncConfig SlopConfigHandle;
int64_t timestamp = 0ll;
int64_t referTimestamp = 0ll;

#define NS (int64_t)1000000000ll
#define TIME_DIFF (int64_t)(1*NS)
#define F_DIFF(data, a, b) (data >= a && data< b) ? 1:0

#define GET_EMPTY_SIZE(len, front, real) ((front) >=(real)) ? (len-(front-real)) : (real-front)

float testSlop = 1.0f;

float currentSlop;

static int slopCallback(double slop)
{
    currentSlop = slop;
    printf("slop update %f", (float)slop);
    return 0;
}

static int64_t getTimestamp(void)
{
    return timestamp;
}

static void timeSyncModuleInit(void)
{
    SlopConfigHandle.minSyncTime = 5*NS;
    SlopConfigHandle.slopMaxTolerance = 0.1f;
    SlopConfigHandle.LOG = printf;
    SlopConfigHandle.getTimestamp = getTimestamp;
    SlopConfigHandle.avargeSlopCallback = slopCallback;
    TimeSlopInit (&SlopHandle, &SlopConfigHandle);

    SlopConfigHandle.offsetMaxTolerance = 2*NS;
    SlopConfigHandle.biasMaxTolerance = 0.1f;
}
 int main( void )
{
    timeSyncModuleInit();
    while(1)
    {
        SlopHandle.inputTime(&SlopHandle, timestamp, referTimestamp);
        timestamp += TIME_DIFF;
        if(timestamp < 100*NS)
            referTimestamp += (TIME_DIFF);
        else  if(F_DIFF(timestamp, 100*NS, 200*NS))
            referTimestamp += (TIME_DIFF*1.1);
        else  if(F_DIFF(timestamp, 200*NS, 300*NS))
            referTimestamp += (TIME_DIFF*1.2);
        else  if(F_DIFF(timestamp, 300*NS, 400*NS))
            referTimestamp += (TIME_DIFF*1.3);
        else  if(F_DIFF(timestamp, 400*NS, 500*NS))
            referTimestamp += (TIME_DIFF*1.4);
        else  if(F_DIFF(timestamp, 500*NS, 600*NS))
            referTimestamp += (TIME_DIFF*0.9);
        else  if(F_DIFF(timestamp, 600*NS, 700*NS))
            referTimestamp += (TIME_DIFF*0.8);
        else  if(F_DIFF(timestamp, 700*NS, 800*NS))
            referTimestamp += (TIME_DIFF*0.7);
        else  if(F_DIFF(timestamp, 10*NS, 900*NS))
            referTimestamp += (TIME_DIFF*0.6);
        else  if(F_DIFF(timestamp, 900*NS, 1000*NS))
            referTimestamp += (TIME_DIFF*0.5);
        else{
            referTimestamp += (TIME_DIFF*testSlop);
            testSlop += 0.000001f;
        }
    }
}

