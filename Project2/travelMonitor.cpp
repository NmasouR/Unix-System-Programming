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
#include <sstream>
#include <sys/wait.h>


#include "travelVirus.h"
#include "queries.h"
#include "signalhandler.h"

#define PERMS 0666

int childTerm = 0;
int term = 0;
pid_t childDied;

void childTerm_handler(int sig){
    int status;
    childDied = waitpid(-1,&status,WNOHANG);
    if(childDied == 0)
        return;
    childTerm = 1;
}

void process_term(int sig){
    term = 1;
}


int init(char** input,int* bloomSize,int* numMonitors,int* bufferSize,char** dirName);

int main(int argc,char* argv[]){
    int bloomSize,numMonitors,bufferSize,error,travelRequest;
    char* dirName;
    pid_t pid;
    DIR* dir;
    struct dirent* entry;
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = childTerm_handler;
    sigaction(SIGCHLD,&sa,NULL);

    struct sigaction sa1;
    memset(&sa1,0,sizeof(sa1));
    sa1.sa_handler = process_term;
    sa1.sa_flags = SA_RESTART;
    sigaction(SIGINT,&sa1,NULL);
    sigaction(SIGQUIT,&sa1,NULL);


    if(argc != 9){
        std::cout << "Wrong params" << std::endl;
        return -1;
    }
    if((error = init(argv,&bloomSize,&numMonitors,&bufferSize,&dirName)) == -1){
        std::cout << "Error in init of program args" << std::endl;
        return -1;
    }
    int writefd[numMonitors]; //write fifos fds
    int readfd[numMonitors];//read fifos fds
    char buffer [numMonitors][bufferSize];//fifo buffer sizes
    char* Rfifos[numMonitors];//travelMonitor reads
    char* Wfifos[numMonitors];//travelMonitor writes
    pid_t monitors[numMonitors];//childs
    travelVirusHead* viruses[numMonitors];
    for(int i = 0;i < numMonitors;i++)
        viruses[i] = new travelVirusHead;

    for(int i = 0;i < numMonitors;i++){ //dynamic initialize fifo names 
        char name[20] = "FIFOR.";
        char index[10];
        sprintf(index,"%d",i);
        strcat(name,index);
        Rfifos[i] = (char*) malloc(sizeof(name));
        strcpy(Rfifos[i],name);
    }

    for(int i = 0;i < numMonitors;i++) //create fifo
        if(mkfifo(Rfifos[i],PERMS) < 0)
            perror("mkfifo");

    for(int i = 0;i < numMonitors;i++){//dynamic initialize fifo names
        char name[20] = "FIFOW.";
        char index[10];
        sprintf(index,"%d",i);
        strcat(name,index);
        Wfifos[i] = (char*) malloc(sizeof(name));
        strcpy(Wfifos[i],name);
    }
    for(int i = 0;i < numMonitors;i++)//create fifo
        if(mkfifo(Wfifos[i],PERMS) < 0)
            perror("mkfifo");
    
    for(int i = 0;i < numMonitors;i++)//open read fifos
        if((readfd[i] = open(Rfifos[i],O_RDONLY | O_NONBLOCK)) < 0){
            perror("open fifo");
            return -1;
        }

    for(int i = 0;i < numMonitors;i++){//create children processes
        pid = fork();
        if(pid == -1){
            perror("fork");
            return -1;
        }
        if(pid == 0){
            char bloom[20];
            char buf[20];
            sprintf(bloom,"%d",bloomSize);
            sprintf(buf,"%d",bufferSize);
            execl("./Monitor",Rfifos[i],Wfifos[i],NULL);
        }
        else{
                monitors[i] = pid;
        }
            
    }
    for(int i = 0;i < numMonitors;i++)//open write fifos
        if((writefd[i] = open(Wfifos[i],O_WRONLY)) < 0){
            perror("open fifo");
            //return -1;
        }
    for(int i = 0;i < numMonitors;i++){
        write(writefd[i],&bufferSize,sizeof(bufferSize));
        write(writefd[i],&bloomSize,sizeof(bloomSize));
        write(writefd[i],dirName,strlen(dirName) + 1);
    }




    if((dir = opendir(dirName)) == NULL){
        perror("opendir");
        return -1;
    }
    int monitor = 0;
    //suppose buffer large enough for all subdirs
    while((entry = readdir(dir)) != NULL){ //send subdirs as round robin to monitors
        if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
            continue;
        if(write(writefd[monitor],entry->d_name,(strlen(entry->d_name) + 1)) < 0){
            perror("write");
            std::cout << "I m here parent" << std::endl;
            std::cout << "couldn't write to monitor" << std::endl;
            return -1;
        }
        monitor++;
        if(monitor == numMonitors)
            monitor = 0;
    }
    closedir(dir);
    for(int i = 0;i < numMonitors;i++)//monitor stop waiting for more dirs
        if(write(writefd[i],"end",(strlen("end") + 1)) != 4){
            std::cout << "couldn't write to monitor" << std::endl;
            return -1;
        }
    //get viruses
    fd_set fds,tempfds;
    int monitorsget = numMonitors,n,msgstart[numMonitors] = {0};
    int maxfd = readfd[0];
    char virus[30];
    FD_ZERO(&fds);
    for(int i = 0;i < numMonitors;i++)
        FD_SET(readfd[i],&fds);//add fds to set for select
    for(int i = 0;i < numMonitors;i++)
        if(readfd[i] > maxfd)
            maxfd = readfd[i];
    while(monitorsget > 0){ //suppose buffer large enough for all viruses names
            tempfds = fds; //because fds are modified by select
            select(maxfd + 1, &tempfds, NULL, NULL, NULL);
            for(int i = 0;i < numMonitors;i++){
                if(FD_ISSET(readfd[i],&tempfds)){
                    n = read(readfd[i],buffer[i],bufferSize);
                    for(int j = 0;j < n;j++){
                        if(buffer[i][j] == '\0'){
                            memcpy(virus,&buffer[i][msgstart[i]],(j - msgstart[i] +1)*sizeof(char));
                            if(!strcmp(virus,"end")){
                                monitorsget--;
                                FD_CLR(readfd[i],&tempfds);
                                FD_CLR(readfd[i],&fds);
                                break;
                            }
                            std::string name;
                            name.assign(virus);
                            travelVirusNode* temp = new travelVirusNode(virus,bloomSize);
                            viruses[i]->addNode(temp);
                            msgstart[i] = j + 1;
                        }
                    }
                }
                msgstart[i] = 0;
            }
    }
    sendCont(monitors,numMonitors);//send signal to childs to cont
    //now we get blooms


    int bytesRem[numMonitors],inserted[numMonitors] = {0};//bytesRem -> bytes to get in order to complete bloomSize
    for(int i = 0;i < numMonitors;i++)
        bytesRem[i] = bloomSize;
    monitorsget = numMonitors;
    FD_ZERO(&fds);
    for(int i = 0;i < numMonitors;i++)
        FD_SET(readfd[i],&fds);//add fds to set for select
    while(monitorsget > 0){
        tempfds = fds; //because fds are modified by select
        select(maxfd + 1, &tempfds, NULL, NULL, NULL);
        for(int i = 0;i < numMonitors;i++){
            if(FD_ISSET(readfd[i],&tempfds)){
                n = read(readfd[i],buffer[i],bufferSize);
                if(n < 0)
                    continue;
                travelVirusNode* node = viruses[i]->getHead();
                for(int j = 0;j < inserted[i];j++){
                    if(node->next != NULL)
                        node = node->next;
                    else{
                        monitorsget--;
                        FD_CLR(readfd[i],&tempfds);
                        FD_CLR(readfd[i],&fds);
                        continue;
                    }
                }
                int index = 0,err = 0;
                if(bytesRem[i] != bloomSize){
                    if(bytesRem[i] <= n){
                        memcpy(&node->bloomptr[bloomSize - bytesRem[i]],buffer[i],(bytesRem[i] * sizeof(char)));
                        index = bytesRem[i];
                        inserted[i]++;
                        bytesRem[i] = bloomSize;
                        if(node->next != NULL)
                            node = node->next;
                        else{
                            monitorsget--;
                            FD_CLR(readfd[i],&tempfds);
                            FD_CLR(readfd[i],&fds);
                            continue;
                        }
                    }
                    else{
                        memcpy(&node->bloomptr[bloomSize - bytesRem[i]],buffer[i],(n * sizeof(char)));
                        index = n;
                        bytesRem[i] = bytesRem[i] - n;
                    }
                }
                for(int k = index;k < n;k+=bloomSize){
                    if((n - k) < bloomSize)
                        break;
                    memcpy(node->bloomptr,&buffer[i][k],bloomSize * sizeof(char));
                    inserted[i]++;
                    if(node->next != NULL)
                        node = node->next;
                    else{
                        monitorsget--;
                        FD_CLR(readfd[i],&tempfds);
                        FD_CLR(readfd[i],&fds);
                        err = 1;
                        break;
                    }
                    index += bloomSize;
                }
                if(index < n && err == 0){
                    memcpy(node->bloomptr,&buffer[i][index],(n - index) * sizeof(char));
                    bytesRem[i] = bytesRem[i] - (n - index);
                    
                }
                memset(buffer[i],0,bufferSize * sizeof(char));
            }
        }
    }
    monitorsget = numMonitors;
    sendCont(monitors,numMonitors);//send signal to childs to cont
    //get countries

    travelCountryHead* countries[numMonitors];
    for(int i = 0;i < numMonitors;i++)
        countries[i] = new travelCountryHead;
    char country[30];
    FD_ZERO(&fds);
    for(int i = 0;i < numMonitors;i++)
        FD_SET(readfd[i],&fds);//add fds to set for select
    while(monitorsget > 0){ //suppose buffer large enough for all country names
            tempfds = fds; //because fds are modified by select
            select(maxfd + 1, &tempfds, NULL, NULL, NULL);
            for(int i = 0;i < numMonitors;i++){
                if(FD_ISSET(readfd[i],&tempfds)){
                    n = read(readfd[i],buffer[i],bufferSize);
                    for(int j = 0;j < n;j++){
                        if(buffer[i][j] == '\0'){
                            memcpy(country,&buffer[i][msgstart[i]],(j - msgstart[i] +1)*sizeof(char));
                            if(!strcmp(country,"end")){
                                monitorsget--;
                                FD_CLR(readfd[i],&tempfds);
                                FD_CLR(readfd[i],&fds);
                                break;
                            }
                            std::string name;
                            name.assign(country);
                            travelCountryNode* temp = new travelCountryNode(country);
                            countries[i]->addNode(temp);
                            msgstart[i] = j + 1;
                        }
                    }
                }
                msgstart[i] = 0;
            }
    }
    if(childTerm == 1){ //Data Remains the same
        //create new Monitor
        int found;
        std::cout << childDied << std::endl;
        for(int i = 0;i < numMonitors;i++){
            if(childDied == monitors[i])
                found = i;
        }
        close(writefd[found]);

        pid = fork();
        if(pid == -1){
            perror("fork");
            return -1;
        }
        if(pid == 0){
            char bloom[20];
            char buf[20];
            sprintf(bloom,"%d",bloomSize);
            sprintf(buf,"%d",bufferSize);
            execl("./Monitor",Rfifos[found],Wfifos[found],NULL);
        }
        else
            monitors[found] = pid;

        if((writefd[found] = open(Wfifos[found],O_WRONLY)) < 0){
            perror("open fifo");
            //return -1;
        }
        write(writefd[found],&bufferSize,sizeof(bufferSize));
        write(writefd[found],&bloomSize,sizeof(bloomSize));
        write(writefd[found],dirName,strlen(dirName) + 1);


        //send countries don't get back blooms they already exists
        //send something first to tell that data already exists
        travelCountryNode* tempCountry = countries[found]->getHead();
        while(tempCountry != NULL){
            write(writefd[found],tempCountry->countryName.c_str(),strlen(tempCountry->countryName.c_str()) + 1);
            tempCountry = tempCountry->next;
        }
        write(writefd[found],"new",strlen("new") + 1);
        write(writefd[found],"end",strlen("end") + 1);
        childTerm = 0;
    }

    if(term == 1){
        sa.sa_handler = SIG_DFL;
        sigaction(SIGCHLD,&sa,NULL);
        for(int i = 0;i < numMonitors;i++){
            kill(monitors[i],SIGKILL);
            wait(NULL);
        }
        printStats(countries,numMonitors);
        for(int i = 0;i < numMonitors;i++){
            delete countries[i];
            delete viruses[i];
            free(Wfifos[i]);
            free(Rfifos[i]);
        }
        return 0;
    }


    std::cout << "Ready to accept input!!" << std::endl;
    //end of initialization
    //now we start getting input commands from user
    std::string input;
	std::string command;
    int answer;
    //remove O_NONBLOCK from readfds
    int oldfd;
    for(int i = 0;i < numMonitors;i++){
        oldfd = fcntl(readfd[i],F_GETFL);
        fcntl(readfd[i], F_SETFL, oldfd & ~O_NONBLOCK);
    }
    do{
        std::cout  << "Give your input:";
		std::getline(std::cin, input);
		std::stringstream ss(input);
		std::getline(ss, command, ' ');
        if(command == "/travelRequest"){

            std::string ID,date,countryFrom,countryTo,virusName;
            std::getline(ss, ID, ' ');
            std::getline(ss, date, ' ');
            std::getline(ss, countryFrom, ' ');
            std::getline(ss, countryTo, ' ');
            std::getline(ss, virusName);
            int found,monitorFound,monitorTo;
            for(int i = 0;i < numMonitors;i++){
                found = findCountry(countryFrom,countries[i]);
                if(found == 1){
                    monitorFound = i;
                    break;
                }
            }
            if(found == 0){
                std::cout << "can't find countryFrom" << std::endl;
                continue;
            }


            for(int i = 0;i < numMonitors;i++){
                found = findCountry(countryTo,countries[i]);
                if(found == 1){
                    monitorTo = i;
                    break;
                }
            }


            if(found == 0){
                std::cout << "can't find countryTo" << std::endl;
                continue;
            }
            found = BloomStatus(ID,virusName,viruses[monitorFound],bloomSize);
            if(found == 0){//not vaccinated
                reverseDate(date);
                request* req = new request(date,0,1,virusName);
                countries[monitorTo]->search(countryTo)->addRequest(req);
                travelRequest++;
                std::cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << std::endl;
                memset(buffer[monitorFound],0,bufferSize * sizeof(char));
            }
            else{//maybe vaccinated
                if(write(writefd[monitorFound],"/travelRequest",strlen("/travelRequest") + 1) < 0){
                    perror("write");
                    return -1;
                }
                if(write(writefd[monitorFound],ID.c_str(),strlen(ID.c_str()) + 1) < 0){
                    perror("write");
                    return -1;
                }
                if(write(writefd[monitorFound],virusName.c_str(),strlen(virusName.c_str()) + 1) < 0){
                    perror("write");
                    return -1;
                }
                if(write(writefd[monitorFound],date.c_str(),strlen(date.c_str()) + 1) < 0){
                    perror("write");
                    return -1;
                }
                sleep(1);
                if( (n = read(readfd[monitorFound],buffer[monitorFound],bufferSize)) < 0){
                    perror("read");
                    return -1;
                }
                answer = travelRequestGet(buffer[monitorFound],date,n);
                if(answer < 0){
                    memset(buffer[monitorFound],0,bufferSize * sizeof(char));
                    std::cout << "wrong date input" << std::endl;
                }
                else if(answer == 0){
                    memset(buffer[monitorFound],0,bufferSize * sizeof(char));
                    reverseDate(date);
                    request* req = new request(date,0,1,virusName);
                    countries[monitorTo]->search(countryTo)->addRequest(req);
                    travelRequest++;
                    std::cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << std::endl;
                }
                else if(answer == 1){
                    memset(buffer[monitorFound],0,bufferSize * sizeof(char));
                    reverseDate(date);
                    request* req = new request(date,1,0,virusName);
                    countries[monitorTo]->search(countryTo)->addRequest(req);
                    travelRequest++;
                    std::cout << "REQUEST ACCEPTED – HAPPY TRAVELS" << std::endl;
                }
                else if(answer == 2){
                    memset(buffer[monitorFound],0,bufferSize * sizeof(char));
                    reverseDate(date);
                    request* req = new request(date,0,1,virusName);
                    countries[monitorTo]->search(countryTo)->addRequest(req);
                    travelRequest++;
                    std::cout << "REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE" << std::endl;
                }
            }
        }
        else if(command == "/travelStats"){
            std::string date1,date2,country,virus;
            int dateerr;
            std::getline(ss,virus,' ');
            std::getline(ss,date1,' ');
            std::getline(ss,date2,' ');
            if((dateerr = reverseDate(date1)) < 0){
                std::cout << "Wrong date format" << std::endl;
                continue;
            }
            if((dateerr = reverseDate(date2)) < 0){
                std::cout << "Wrong date format" << std::endl;
                continue;
            }
            std::getline(ss,country);
            if(country.length() == 0){//no country input
                statsAllCountries(countries,date1,date2,numMonitors,virus);
            }
            else
                statsCountry(countries,country,date1,date2,numMonitors,virus);
        }
        else if(command == "/addVaccinationRecords"){
            std::string country;
            char virusName[30];
            int found,monitorFound;
            std::getline(ss,country);
            for(int i = 0;i < numMonitors;i++){
                found = findCountry(country,countries[i]);
                if(found == 1){
                    monitorFound = i;
                    break;
                }
            }
            if(found == 0){
                std::cout << "Country " << country << " doesn't exists" << std::endl;
                continue;
            }
            kill(monitors[monitorFound],SIGUSR1);
            delete viruses[monitorFound];
            viruses[monitorFound] = new travelVirusHead;
            int start1 = 0;
            int end = 0;
            //get new data
            while(true){//get viruses
                if((n = read(readfd[monitorFound],buffer[monitorFound],bufferSize)) < 0){
                    perror("read");
                    return -1;
                }   
                for(int j = 0;j < n;j++){
                    if(buffer[monitorFound][j] == '\0'){
                        memcpy(virusName,&buffer[monitorFound][start1],(j - start1 +1)*sizeof(char));
                        if(!strcmp(virusName,"end")){
                            end = 1;
                            break;
                        }
                        std::string name;
                        name.assign(virusName);
                        travelVirusNode* temp = new travelVirusNode(virusName,bloomSize);
                        viruses[monitorFound]->addNode(temp);
                        start1 = j + 1;
                    }
                }
                if(end == 1)
                    break;
                start1 = 0;
            }   
            kill(monitors[monitorFound],SIGCONT);
            travelVirusNode* node = viruses[monitorFound]->getHead();
            int insert=0;
            int bytes = bloomSize;
            while(true){//get blooms
                if((n = read(readfd[monitorFound],buffer[monitorFound],bufferSize)) < 0){
                    perror("read");
                    return -1;
                }
                int index = 0,err = 0;
                if(insert == 1){
                    if(node->next != NULL){
                        node = node->next;
                        insert = 0;
                    }
                    else break;
                }
                if(bytes != bloomSize){
                    if(bytes <= n){
                        memcpy(&node->bloomptr[bloomSize - bytes],buffer[monitorFound],bytes * sizeof(char));
                        insert = 1;
                        index = bytes;
                        bytes = bloomSize;
                        if(node->next != NULL){
                            node = node->next;
                            insert = 0;
                        }
                        else
                            break;
                    }
                    else{
                        memcpy(&node->bloomptr[bloomSize - bytes],buffer[monitorFound],n * sizeof(char));
                        index = n;
                        bytes = bytes - n;
                    }
                }
                for(int k = index;k < n;k+=bloomSize){
                    if((n - k) < bloomSize)
                        break;
                    memcpy(node->bloomptr,&buffer[monitorFound][k],bloomSize * sizeof(char));
                    insert = 1;
                    if(node->next != NULL){
                        node = node->next;
                        insert = 0;
                    }
                    else{
                        err =1;
                        break;
                    }
                        
                    index += bloomSize;
                }
                if(err == 1)
                    break;
                if(index < n){
                    memcpy(node->bloomptr,&buffer[monitorFound][index],(n - index) * sizeof(char));
                    bytes = bytes - (n - index);
                }
                memset(buffer[monitorFound],0,bufferSize * sizeof(char));
            }
            std::cout << "Done!!" << std::endl;
        }
        else if(command == "/searchVaccinationStatus"){
            std::string ID,firstName,lastName,age,vaccinated;
            std::getline(ss,ID);
            for(int i = 0;i < numMonitors;i++){
                write(writefd[i],"/searchVaccinationStatus",strlen("/searchVaccinationStatus") + 1);
                write(writefd[i],ID.c_str(),strlen(ID.c_str()) + 1);
            }
            sleep(1);
            for(int i = 0;i < numMonitors;i++){
                if((n = read(readfd[i],buffer[i],bufferSize)) < 0){
                    perror("read");
                    return -1;
                }
                GetVaccinationStatus(buffer[i],ID,n,readfd[i]);
            }
        }
        if(command == "/exit" || term == 1){
            std::cout << "/exit" << std::endl;
            sa.sa_handler = SIG_DFL;
            sigaction(SIGCHLD,&sa,NULL);
            for(int i = 0;i < numMonitors;i++){
                kill(monitors[i],SIGKILL);
                wait(NULL);
            }
            printStats(countries,numMonitors);
            for(int i = 0;i < numMonitors;i++){
                delete countries[i];
                delete viruses[i];
                free(Wfifos[i]);
                free(Rfifos[i]);
            }
            break;    
        }
        if(childTerm == 1){ //Data Remains the same
            //create new Monitor
            int found;
            for(int i = 0;i < numMonitors;i++){
                if(childDied == monitors[i])
                    found = i;
            }
            close(writefd[found]);

            pid = fork();
            if(pid == -1){
                perror("fork");
                return -1;
            }
            else if(pid == 0){
                char bloom[20];
                char buf[20];
                sprintf(bloom,"%d",bloomSize);
                sprintf(buf,"%d",bufferSize);
                execl("./Monitor",Rfifos[found],Wfifos[found],NULL);
            }
            else{
                monitors[found] = pid;
                if((writefd[found] = open(Wfifos[found],O_WRONLY)) < 0){
                    perror("open fifo");
                    //return -1;
                }
                write(writefd[found],&bufferSize,sizeof(bufferSize));
                write(writefd[found],&bloomSize,sizeof(bloomSize));
                write(writefd[found],dirName,strlen(dirName) + 1);
                //send countries don't get back blooms they already exists
                //send something first to tell that data already exists
                travelCountryNode* tempCountry = countries[found]->getHead();
                while(tempCountry != NULL){
                    write(writefd[found],tempCountry->countryName.c_str(),strlen(tempCountry->countryName.c_str()) + 1);
                    tempCountry = tempCountry->next;
                }
                write(writefd[found],"new",strlen("new") + 1);
                write(writefd[found],"end",strlen("end") + 1);
                childTerm = 0;
                std::cin.clear();
                fflush(stdin);
                std::cout <<"New Monitor created" << std::endl;
            }
        } 
    }while(true);

    return 0;
}

int init(char** input,int* bloomSize,int* numMonitors,int* bufferSize,char** dirName){
    for(int i = 1;i <= 7;i += 2){
        if(!(strcmp(input[i], "-m"))){
            *numMonitors = atoi(input[i+1]);
        }
        else if(!(strcmp(input[i], "-b"))){
            *bufferSize = atoi(input[i+1]);
        }
        else if(!(strcmp(input[i], "-s"))){
            *bloomSize = atoi(input[i+1]);
        }
        else if(!(strcmp(input[i], "-i"))){
            *dirName = input[i+1];
        }
        else{
            std::cout <<"Wrong params"<< std::endl;
            return -1;
        }
    }
    return 0;
}
