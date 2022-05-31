#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <dirent.h>
#include <sys/stat.h> //mkfifo
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "MonitorGet.h"
#include "MonitorData.h"

int added = 0;
int term = 0;

void USR1_handler(int sig){
    added = 1;
}

void term_handler(int sig){
    term = 1;
}

int main(int argc,char* argv[]){
    int writefd,readfd,msgstart = 0,n,dataEx = 0;
    virusHead* viruses = new virusHead;
	citizenHead* citizens = new citizenHead;
	countryHead* countries = new countryHead;
    if((writefd = open(argv[0],O_WRONLY)) < 0){
        perror("open fifo");
        delete viruses;
        delete citizens;
        delete countries;
        exit(-1);
    }
    if((readfd = open(argv[1],O_RDONLY)) < 0){
        perror("open fifo");
        delete viruses;
        delete citizens;
        delete countries;
        exit(-1);
    }
    int bloomSize,bufferSize;
    read(readfd,&bufferSize,sizeof(bufferSize));
    read(readfd,&bloomSize,sizeof(bloomSize));


    char buffer[bufferSize];
    char country[30];
    int msgstart1=0;
    n = read(readfd,buffer,bufferSize);
    std::string dirname;
    dirname = getString(buffer,&msgstart1,n);
    const char* directory = dirname.c_str();

    struct sigaction sa1;
    memset(&sa1,0,sizeof(sa1));
    sa1.sa_handler = term_handler;
    sa1.sa_flags = SA_RESTART;
    sigaction(SIGINT,&sa1,NULL);
    sigaction(SIGQUIT,&sa1,NULL);
    while(true){ //reading directories
        if((n = read(readfd,buffer,bufferSize)) < 0){ 
            perror("read dir");
            delete viruses;
            delete citizens;
            delete countries;
            exit(-1);
        }
        for(int i = 0;i < n;i++){
            if(buffer[i] == '\0'){
                memcpy(country,&buffer[msgstart],(i - msgstart +1)*sizeof(char));
                if(strlen(country) == 1)
                    continue;
                if(!strcmp(country,"end")){
                    break;
                }
                else if(!strcmp(country,"new")){
                    dataEx = 1;
                }
                else if(getdir(country,viruses,citizens,countries,bloomSize,directory) == -1){
                    std::cout << "can't init dir" << std::endl;
                    exit(-1);
                }
                msgstart = i + 1;
            }
        }
        if(!strcmp(country,"end")){
            break;
        }
        msgstart = 0;
    }
    if(dataEx == 0){
        if(sendvirus(writefd,viruses) < 0){
            std::cout << "can't send virus" << std::endl;
            exit(-1);
        }
        //here I must get a signal that says parent is ready
        kill(getpid(),SIGSTOP);
        if(sendbloom(writefd,viruses,bloomSize) < 0){
            std::cout << "can't send bloom" << std::endl;
            exit(-1);
        }
        //here I must get a signal that says parent is ready
        kill(getpid(),SIGSTOP);
        if(sendcountries(writefd,countries) < 0){
            std::cout << "can't send countries" << std::endl;
            exit(-1);
        }   
    }
    if(term == 1){
        printMonitorStats(countries);
        delete countries;
        delete viruses;
        delete citizens;
        _exit(1);
    }

    memset(buffer,0,bufferSize);
    msgstart = 0;
    char command[30];
    std::string strcommand;
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = USR1_handler;
    sa1.sa_handler = term_handler;
    sigaction(SIGUSR1,&sa,NULL);
    sa1.sa_flags &= ~SA_RESTART;
    sigaction(SIGQUIT,&sa1,NULL);
    sigaction(SIGINT,&sa1,NULL);
    while(true){    
        if((n = read(readfd,buffer,bufferSize * sizeof(char))) < 0){
            if(added == 1 && errno == EINTR){
                delete viruses;
                viruses = new virusHead;
                delete citizens;
                citizens = new citizenHead;
                if(fileAdd(viruses,citizens,countries,bloomSize,directory) < 0){
                    std::cout << "ERROR FINDING NEW FILES" << std::endl;
                    exit(-1);
                }
                if(sendvirus(writefd,viruses) < 0){
                    std::cout << "can't send virus" << std::endl;
                    exit(-1);
                }
                //here I must get a signal that says parent is ready
                kill(getpid(),SIGSTOP);
                if(sendbloom(writefd,viruses,bloomSize) < 0){
                    std::cout << "can't send bloom" << std::endl;
                    exit(-1);
                }
                added = 0;
                continue;
            }
            else if(term == 1 && errno == EINTR){
                printMonitorStats(countries);
                delete countries;
                delete viruses;
                delete citizens;
                close(writefd);
                close(readfd);
                _exit(1);
            }
            else{
                perror("read");
                exit(-1);
            }
        }
        if(n > 0){
            for(int i = 0;i < n;i++){
                if(buffer[i] == '\0'){
                    memcpy(command,buffer,i + 1);
                    msgstart = i + 1;
                    break;
                }
            }
            strcommand.assign(command);
        }
        if(strcommand == "/travelRequest"){//must send country to for log file
            if(n == (strlen("/travelRequest") + 1)){
                memset(buffer,0,bufferSize);
                if((n = read(readfd,buffer,bufferSize * sizeof(char))) < 0){
                    perror("read");
                    exit(-1);
                }
                msgstart = 0;
            }
            std::string ID,virusName,datevaccinated,dateTravel;
            ID = getString(buffer,&msgstart,n);
            if(msgstart == 0){
                memset(buffer,0,bufferSize);
                if((n = read(readfd,buffer,bufferSize * sizeof(char))) < 0){
                    perror("read");
                    exit(-1);
                }
            }
            virusName = getString(buffer,&msgstart,n);
            if(msgstart == 0){
                if((n = read(readfd,buffer,bufferSize * sizeof(char))) < 0){
                    perror("read");
                    exit(-1);
                }
            }
            dateTravel = getString(buffer,&msgstart,n);
            datevaccinated = vaccineStatus(ID,virusName,viruses);
            if(datevaccinated == "0"){
                citizens->searchCitizen(ID)->country->rejected++;
                write(writefd,"NO",strlen("NO") + 1);
            }
            else{
                reverseDate(dateTravel);
                if(datecmp(dateTravel,datevaccinated))
                    citizens->searchCitizen(ID)->country->accepted++;
                else
                    citizens->searchCitizen(ID)->country->rejected++;
                write(writefd,"YES",strlen("YES") + 1);
                write(writefd,datevaccinated.c_str(),strlen(datevaccinated.c_str()) + 1);
                memset(buffer,0,bufferSize * sizeof(char));
            }
        }
        else if(strcommand == "/searchVaccinationStatus"){
            if(n == (strlen("/searchVaccinationStatus") + 1)){
                if((n = read(readfd,buffer,bufferSize * sizeof(char))) < 0){
                    perror("read");
                    exit(-1);
                }
                msgstart = 0;
            }
            std::string ID;
            citizenNode* citizen;
            virusNode* temp;
            ID = getString(buffer,&msgstart,n);
            if((citizen = citizens->searchCitizen(ID)) == NULL){
                write(writefd,"end",strlen("end") + 1);
            }
            else{
                char ageStr[30];
                std::string date;
                sprintf(ageStr,"%d",citizen->age);
                write(writefd,citizen->firstName.c_str(),strlen(citizen->firstName.c_str()) + 1);
                write(writefd,citizen->lastName.c_str(),strlen(citizen->lastName.c_str()) + 1);
                write(writefd,citizen->country->countryName.c_str(),strlen(citizen->country->countryName.c_str()) + 1);
                write(writefd,ageStr,strlen(ageStr) + 1);
                temp = viruses->getHead();
                while(temp != NULL){
                    date = vaccineCitizenStatus(ID,temp->virusName,viruses);
                    write(writefd,temp->virusName.c_str(),strlen(temp->virusName.c_str()) + 1);
                    if(date == "0"){
                        write(writefd,"NO",strlen("NO") + 1);
                    }
                    else{
                        write(writefd,date.c_str(),strlen(date.c_str()) + 1);
                    }
                    temp = temp->next;
                }
                write(writefd,"end",strlen("end") + 1);
            }
        }

        msgstart = 0;
        memset(buffer,0,bufferSize * sizeof(char));
        if(term == 1){
            printMonitorStats(countries);
            delete countries;
            delete viruses;
            delete citizens;
            close(writefd);
            close(readfd);
            _exit(1);
        }
        if(added == 1){
            delete viruses;
            viruses = new virusHead;
            delete citizens;
            citizens = new citizenHead;
            if(fileAdd(viruses,citizens,countries,bloomSize,directory) < 0){
                std::cout << "ERROR FINDING NEW FILES" << std::endl;
                exit(-1);
            }
            if(sendvirus(writefd,viruses) < 0){
                std::cout << "can't send virus" << std::endl;
                exit(-1);
            }
            //here I must get a signal that says parent is ready
            kill(getpid(),SIGSTOP);
            if(sendbloom(writefd,viruses,bloomSize) < 0){
                std::cout << "can't send bloom" << std::endl;
                exit(-1);
            }
            added = 0;
        }
    } 
}