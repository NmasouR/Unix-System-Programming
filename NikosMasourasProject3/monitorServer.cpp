#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netinet/in.h>


#include "MonitorGet.h"
#include "MonitorData.h"
#include "thread.h"


pthread_mutex_t cyclicBufmtx;
pthread_cond_t nonempty;
pthread_cond_t nonfull;


int cyclicBufItems;



int main(int argc,char* argv[])
{
    int port,numThreads,bloomSize,bufferSize,cyclicSize,error = 0,sockfd,connection;
    port = atoi(argv[1]);
    numThreads = atoi(argv[3]);
    bloomSize = atoi(argv[9]);
    bufferSize = atoi(argv[5]);
    cyclicSize = atoi(argv[7]);
    char* countryPaths = argv[10];
    char buffer[bufferSize];
    char countriesArr[150][50];//max 150 countries per monitor
    int countryindex = 0;
    virusHead* viruses = new virusHead;
	citizenHead* citizens = new citizenHead;
	countryHead* countries = new countryHead;
    //create cyclic buffer
    cyclicBuf* cyclBuffer = new cyclicBuf;
    cyclBuffer->size = 0;
    cyclBuffer->start = NULL;
    cyclBuffer->end = NULL;
    //find countries
    for(int i=0;i < 2000;i++){
        if(countryPaths[i] == '.'){
            for(int j = i + 1;j < 2000;j++){
                if(countryPaths[j] == '.' ||countryPaths[j] == '\0'){
                    strncpy(countriesArr[countryindex],&countryPaths[i],j - i);
                    countriesArr[countryindex][j - i ] = '\0';
                    countryindex++;//monitor has countryindex countries
                    if(countryPaths[j] == '\0')
                        error = 1;
                    i = j - 1;
                    break;
                }
            }
        }
        if(error == 1)
            break;
    }
    error = 0;
    //threads Params
    threadParams* params = new threadParams;
    params->bloomSize = bloomSize;
    params->citizens = citizens;
    params->countries = countries;
    params->viruses = viruses;
    params->cyclicBuffer = cyclBuffer;

    pthread_mutex_init(&cyclicBufmtx,0);
    pthread_cond_init(&nonempty,0);
    pthread_cond_init(&nonfull,0);

    pthread_t threads[numThreads];

    cyclicBufItems = countryindex;
    for(int i = 0;i < numThreads;i++)
        if((error = pthread_create(&threads[i],NULL,monitorThread,params))){//args are cyclicsize bloomsize for sure and we see
            std::cout << "Error Creating Thread" << std::endl;
            exit(-1);
        }
    
    int index = 0;
    //initialize Data with Cyclic Buffer
    while(index < countryindex){
        if (getdir(countriesArr[index],cyclBuffer,cyclicSize) < 0){
            std::cout << "Can't open directory" << std::endl;
            exit(-1);
        }
        index++;
        cyclicBufItems--;
    }
    //find host IP
    char hostbuf[256];
    int hostname = gethostname(hostbuf,sizeof(hostbuf));
    if(hostname == -1){
        perror("gethostname");
        exit(1);
    }
    struct hostent *host_entry;
    host_entry = gethostbyname(hostbuf);
    if(host_entry == NULL){
        perror("gethostbyname");
        exit(-1);
    }


    struct sockaddr_in sock,cli;
    socklen_t clientlen;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed\n");
        exit(0);
    }
    //socket init
    sock.sin_family = AF_INET;
    memcpy(&sock.sin_addr,host_entry->h_addr,host_entry->h_length);
    sock.sin_port = htons(port);

    if ((bind(sockfd,(sockaddr*)&sock, sizeof(sock))) != 0) {
        close(sockfd);
        perror("bind");
        printf("socket bind failed\n");
        exit(0);
    }
    if ((listen(sockfd, 1)) != 0) {
        printf("Listen failed\n");
        exit(0);
    }
    connection = accept(sockfd,(sockaddr*)&cli,&clientlen);

    if(sendvirus(connection,viruses) < 0){
        std::cout << "can't send virus" << std::endl;
        exit(-1);
    }
    //here I must get a signal that says parent is ready
    kill(getpid(),SIGSTOP);
    if(sendbloom(connection,viruses,bloomSize) < 0){
        std::cout << "can't send bloom" << std::endl;
        exit(-1);
    }
    //here I must get a signal that says parent is ready
    kill(getpid(),SIGSTOP);
    if(sendcountries(connection,countries) < 0){
        std::cout << "can't send countries" << std::endl;
        exit(-1);
    }

    memset(buffer,0,bufferSize);
    int msgstart = 0;
    char command[30];
    std::string strcommand;
    int n;
    while(true){    
        n = read(connection,buffer,bufferSize * sizeof(char));
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
                if((n = read(connection,buffer,bufferSize * sizeof(char))) < 0){
                    perror("read");
                    exit(-1);
                }
                msgstart = 0;
            }
            std::string ID,virusName,datevaccinated,dateTravel;
            ID = getString(buffer,&msgstart,n);
            if(msgstart == 0){
                memset(buffer,0,bufferSize);
                if((n = read(connection,buffer,bufferSize * sizeof(char))) < 0){
                    perror("read");
                    exit(-1);
                }
            }
            virusName = getString(buffer,&msgstart,n);
            if(msgstart == 0){
                if((n = read(connection,buffer,bufferSize * sizeof(char))) < 0){
                    perror("read");
                    exit(-1);
                }
            }
            dateTravel = getString(buffer,&msgstart,n);
            datevaccinated = vaccineStatus(ID,virusName,viruses);
            if(datevaccinated == "0"){
                citizens->searchCitizen(ID)->country->rejected++;
                write(connection,"NO",strlen("NO") + 1);
            }
            else{
                reverseDate(dateTravel);
                if(datecmp(dateTravel,datevaccinated))
                    citizens->searchCitizen(ID)->country->accepted++;
                else
                    citizens->searchCitizen(ID)->country->rejected++;
                write(connection,"YES",strlen("YES") + 1);
                write(connection,datevaccinated.c_str(),strlen(datevaccinated.c_str()) + 1);
                memset(buffer,0,bufferSize * sizeof(char));
            }
        }
        else if(strcommand == "/addVaccinationRecords"){
            delete viruses;
            viruses = new virusHead;
            delete citizens;
            citizens = new citizenHead;
            cyclBuffer->size = 0;
            cyclBuffer->start = NULL;
            cyclBuffer->end = NULL;
            cyclicBufItems = countryindex;
            params->citizens = citizens;
            params->viruses = viruses;
            index = 0;
            for(int i = 0;i < numThreads;i++)
            if((error = pthread_create(&threads[i],NULL,monitorThread,params))){
                std::cout << "Error Creating Thread" << std::endl;
                exit(-1);
            }
        
            while(index < countryindex){
                if (getdir(countriesArr[index],cyclBuffer,cyclicSize) < 0){
                    std::cout << "Can't open directory" << std::endl;
                    exit(-1);
                }
                index++;
                cyclicBufItems--;
            }
            if(sendvirus(connection,viruses) < 0){
                std::cout << "can't send virus" << std::endl;
                exit(-1);
            }
            //here I must get a signal that says parent is ready
            kill(getpid(),SIGSTOP);
            if(sendbloom(connection,viruses,bloomSize) < 0){
                std::cout << "can't send bloom" << std::endl;
                exit(-1);
            }
            continue;
        }
        else if(strcommand == "/searchVaccinationStatus"){
            if(n == (strlen("/searchVaccinationStatus") + 1)){
                if((n = read(connection,buffer,bufferSize * sizeof(char))) < 0){
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
                write(connection,"end",strlen("end") + 1);
            }
            else{
                char ageStr[30];
                std::string date;
                sprintf(ageStr,"%d",citizen->age);
                write(connection,citizen->firstName.c_str(),strlen(citizen->firstName.c_str()) + 1);
                write(connection,citizen->lastName.c_str(),strlen(citizen->lastName.c_str()) + 1);
                write(connection,citizen->country->countryName.c_str(),strlen(citizen->country->countryName.c_str()) + 1);
                write(connection,ageStr,strlen(ageStr) + 1);
                temp = viruses->getHead();
                while(temp != NULL){
                    date = vaccineCitizenStatus(ID,temp->virusName,viruses);
                    write(connection,temp->virusName.c_str(),strlen(temp->virusName.c_str()) + 1);
                    if(date == "0"){
                        write(connection,"NO",strlen("NO") + 1);
                    }
                    else{
                        write(connection,date.c_str(),strlen(date.c_str()) + 1);
                    }
                    temp = temp->next;
                }
                write(connection,"end",strlen("end") + 1);
            }
        }
        else if(strcommand == "/exit"){
            printMonitorStats(countries);
            delete countries;
            delete viruses;
            delete citizens;
            delete params;
            close(connection);
            close(sockfd);
            break;
        }

        msgstart = 0;
        memset(buffer,0,bufferSize * sizeof(char));
    }
    exit(0);
    
}

