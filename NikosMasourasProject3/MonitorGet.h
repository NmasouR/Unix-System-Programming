#pragma once

#include "virusList.h"
#include "citizen.h"

struct cyclicBufData{
    std::string file;
    cyclicBufData* next;
};


struct cyclicBuf{
    int size;
    struct cyclicBufData* start;
    struct cyclicBufData* end;
    void addNode(struct cyclicBufData* node);

};


extern pthread_mutex_t cyclicBufmtx;
extern pthread_cond_t nonempty;
extern pthread_cond_t nonfull;


int getdir(char* dirName,struct cyclicBuf* buffer,int cyclicSize);

int sendbloom(int writefd,virusHead* viruses,int bloomsize);

int sendvirus(int writefd,virusHead* viruses);

int sendcountries(int writefd,countryHead* countries);

std::string getString(char *buffer,int* msgstart,int n);