#include <signal.h>
#include "signalhandler.h"
#include <sys/wait.h>

void sendCont(int *pid,int num){
    for(int i = 0;i < num;i++){
        kill(pid[i],SIGCONT);
    }
}
