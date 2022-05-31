#include "thread.h"
#include <unistd.h>

void *monitorThread(void* params){
    std::string filePath;
    cyclicBuf* cyclicBuffer = (*(threadParams*) params).cyclicBuffer;
    virusHead* viruses =  (*(threadParams*) params).viruses;
    citizenHead* citizens =  (*(threadParams*) params).citizens;
    countryHead* countries =  (*(threadParams*) params).countries;
    int bloomSize =  (*(threadParams*) params).bloomSize;
    cyclicBufData* temp;
        while(cyclicBufItems > 0 || cyclicBuffer->size > 0){
            pthread_mutex_lock(&cyclicBufmtx);
            while (cyclicBuffer->size <= 0) {
                pthread_cond_wait(&nonempty, &cyclicBufmtx);
            }
            filePath = cyclicBuffer->start->file;
            temp = cyclicBuffer->start;
            cyclicBuffer->start = cyclicBuffer->start->next;
            delete temp;
            cyclicBuffer->size--;
            if(inputFromFile(filePath,*viruses,*citizens,*countries,bloomSize) < 0){
                std::cout << "Can't init file" << std::endl;
                exit(-1);
            }
            pthread_cond_signal(&nonfull);
            pthread_mutex_unlock(&cyclicBufmtx);
            usleep(5000);
        }
    pthread_exit(NULL);
}