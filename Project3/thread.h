#pragma once

#include "MonitorData.h"
#include "MonitorGet.h"

extern pthread_mutex_t cyclicBufmtx;
extern pthread_cond_t nonempty;
extern pthread_cond_t nonfull;
extern int cyclicBufItems;
extern int threadExit;

struct threadParams{
    int bloomSize;
    int cyclicSize;
    virusHead* viruses;
    citizenHead* citizens;
    countryHead* countries;
    cyclicBuf* cyclicBuffer;
};

void *monitorThread(void* params);
