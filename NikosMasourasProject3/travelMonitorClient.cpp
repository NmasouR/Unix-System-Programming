#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <netinet/in.h>
#include <wait.h>

#include "travelVirus.h"
#include "queries.h"


int init(char** input,int* bloomSize,int* numMonitors,int* bufferSize,int* cyclicBuffer,int* numThreads,char** dirName);

void sendCont(int *pid,int num){
    for(int i = 0;i < num;i++){
        kill(pid[i],SIGCONT);
    }
}



int main(int argc,char* argv[])
{
    int bloomSize,bufferSize,cyclicBuffer ,numThreads,numMonitors,error,travelRequest;
    char* dirName;
    DIR* dir;
    struct dirent* entry;
    pid_t pid;
    //start of free ports
    int lower = 49152;

    if(argc != 13){
        std::cout << "Wrong params" << std::endl;
        return -1;
    }
    if((error = init(argv,&bloomSize,&numMonitors,&bufferSize,&cyclicBuffer,&numThreads,&dirName)) == -1){
        std::cout << "Error in init of program args" << std::endl;
        return -1;
    }
    pid_t monitors[numMonitors];
    char buffer[numMonitors][bufferSize];
    for(int i = 0;i < numMonitors;i++)
        bzero(buffer[i], sizeof(buffer[i]));
    travelVirusHead* viruses[numMonitors];
    for(int i = 0;i < numMonitors;i++)
        viruses[i] = new travelVirusHead;

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
    int sockfds[numMonitors];
    struct sockaddr_in socks[numMonitors];

    //create sockets
    for(int i = 0;i < numMonitors;i++){
        sockfds[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfds[i] == -1) {
        printf("socket creation failed\n");
        exit(0);
        }
    }
    for(int i = 0; i < numMonitors;i++)
        bzero(&socks[i], sizeof(socks[i]));

    for(int i=0;i < numMonitors;i++){
        socks[i].sin_family = AF_INET;
        memcpy(&socks[i].sin_addr,host_entry->h_addr,host_entry->h_length);
        socks[i].sin_port = htons(lower);
        lower++;
    }

    if((dir = opendir(dirName)) == NULL){
        perror("opendir");
        return -1;
    }
    int monitor = 0;
    char countryNames[numMonitors][2000];
    for(int i = 0;i < numMonitors;i++)
        memset(countryNames[i],0,sizeof(countryNames[i]));

    char dirpath[50];
    strcpy(dirpath,"./");
    strcat(dirpath,dirName);
    strcat(dirpath,"/");

    while((entry = readdir(dir)) != NULL){ //send subdirs as round robin to monitors
        if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
            continue;
        strcat(countryNames[monitor],dirpath);
        strcat(countryNames[monitor],entry->d_name);
        monitor++;
        if(monitor == numMonitors)
            monitor = 0;
    }
    closedir(dir);
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
            char threads[20];
            sprintf(threads,"%d",numThreads);
            char cyclic[20];
            sprintf(cyclic,"%d",cyclicBuffer);
            char portChar[20];
            unsigned short portNum = ntohs(socks[i].sin_port);
            sprintf(portChar,"%d",portNum);
            execlp("./monitorServer","-p",portChar,"-t",threads,"-b",buf,"-c",cyclic,"-s",bloom,countryNames[i],NULL);
        }
        else{
            monitors[i] = pid;
        }
    }
    sleep(1);
    for(int i = 0;i < numMonitors;i++){
        if (connect(sockfds[i],(sockaddr*)&socks[i],sizeof(socks[i])) != 0) {
            close(sockfds[i]);
            perror("connect");
            printf("connection with the server failed...\n");
            exit(0);
        }
    }

    //get viruses
    fd_set fds,tempfds;
    int monitorsget = numMonitors,n,msgstart[numMonitors] = {0};
    int maxfd = sockfds[0];
    char virus[30];
    FD_ZERO(&fds);
    for(int i = 0;i < numMonitors;i++)
        FD_SET(sockfds[i],&fds);//add fds to set for select
    for(int i = 0;i < numMonitors;i++)
        if(sockfds[i] > maxfd)
            maxfd = sockfds[i];
    while(monitorsget > 0){ //suppose buffer large enough for all viruses names
            tempfds = fds; //because fds are modified by select
            select(maxfd + 1, &tempfds, NULL, NULL, NULL);
            for(int i = 0;i < numMonitors;i++){
                if(FD_ISSET(sockfds[i],&tempfds)){
                    n = read(sockfds[i],buffer[i],bufferSize);
                    for(int j = 0;j < n;j++){
                        if(buffer[i][j] == '\0'){
                            memcpy(virus,&buffer[i][msgstart[i]],(j - msgstart[i] +1)*sizeof(char));
                            if(!strcmp(virus,"end")){
                                monitorsget--;
                                FD_CLR(sockfds[i],&tempfds);
                                FD_CLR(sockfds[i],&fds);
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

    int bytesRem[numMonitors],inserted[numMonitors] = {0};//bytesRem -> bytes to get in order to complete bloomSize
    for(int i = 0;i < numMonitors;i++)
        bytesRem[i] = bloomSize;
    monitorsget = numMonitors;
    FD_ZERO(&fds);
    for(int i = 0;i < numMonitors;i++)
        FD_SET(sockfds[i],&fds);//add fds to set for select
    while(monitorsget > 0){
        tempfds = fds; //because fds are modified by select
        select(maxfd + 1, &tempfds, NULL, NULL, NULL);
        for(int i = 0;i < numMonitors;i++){
            if(FD_ISSET(sockfds[i],&tempfds)){
                n = read(sockfds[i],buffer[i],bufferSize);
                if(n < 0)
                    continue;
                travelVirusNode* node = viruses[i]->getHead();
                for(int j = 0;j < inserted[i];j++){
                    if(node->next != NULL)
                        node = node->next;
                    else{
                        monitorsget--;
                        FD_CLR(sockfds[i],&tempfds);
                        FD_CLR(sockfds[i],&fds);
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
                            FD_CLR(sockfds[i],&tempfds);
                            FD_CLR(sockfds[i],&fds);
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
                        FD_CLR(sockfds[i],&tempfds);
                        FD_CLR(sockfds[i],&fds);
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
        FD_SET(sockfds[i],&fds);//add fds to set for select
    while(monitorsget > 0){ //suppose buffer large enough for all country names
            tempfds = fds; //because fds are modified by select
            select(maxfd + 1, &tempfds, NULL, NULL, NULL);
            for(int i = 0;i < numMonitors;i++){
                if(FD_ISSET(sockfds[i],&tempfds)){
                    n = read(sockfds[i],buffer[i],bufferSize);
                    for(int j = 0;j < n;j++){
                        if(buffer[i][j] == '\0'){
                            memcpy(country,&buffer[i][msgstart[i]],(j - msgstart[i] +1)*sizeof(char));
                            if(!strcmp(country,"end")){
                                monitorsget--;
                                FD_CLR(sockfds[i],&tempfds);
                                FD_CLR(sockfds[i],&fds);
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
    std::cout << "Ready to accept input!!" << std::endl;
    std::string input;
	std::string command;
    int answer;

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
                if(write(sockfds[monitorFound],"/travelRequest",strlen("/travelRequest") + 1) < 0){
                    perror("write");
                    return -1;
                }
                if(write(sockfds[monitorFound],ID.c_str(),strlen(ID.c_str()) + 1) < 0){
                    perror("write");
                    return -1;
                }
                if(write(sockfds[monitorFound],virusName.c_str(),strlen(virusName.c_str()) + 1) < 0){
                    perror("write");
                    return -1;
                }
                if(write(sockfds[monitorFound],date.c_str(),strlen(date.c_str()) + 1) < 0){
                    perror("write");
                    return -1;
                }
                sleep(1);
                if( (n = read(sockfds[monitorFound],buffer[monitorFound],bufferSize)) < 0){
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
            write(sockfds[monitorFound],"/addVaccinationRecords",strlen("/addVaccinationRecords") + 1);
            delete viruses[monitorFound];
            viruses[monitorFound] = new travelVirusHead;
            int start1 = 0;
            int end = 0;
            sleep(1);
            //get new data
            while(true){//get viruses
                if((n = read(sockfds[monitorFound],buffer[monitorFound],bufferSize)) < 0){
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
                if((n = read(sockfds[monitorFound],buffer[monitorFound],bufferSize)) < 0){
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
                write(sockfds[i],"/searchVaccinationStatus",strlen("/searchVaccinationStatus") + 1);
                write(sockfds[i],ID.c_str(),strlen(ID.c_str()) + 1);
            }
            sleep(1);
            for(int i = 0;i < numMonitors;i++){
                if((n = read(sockfds[i],buffer[i],bufferSize)) < 0){
                    perror("read");
                    return -1;
                }
                GetVaccinationStatus(buffer[i],ID,n,sockfds[i]);
            }
        }
        if(command == "/exit"){
            for(int i = 0;i < numMonitors;i++){
                write(sockfds[i],"/exit",strlen("/exit") + 1);
                wait(NULL);
            }
            printStats(countries,numMonitors);
            for(int i = 0;i < numMonitors;i++){
                delete countries[i];
                delete viruses[i];
                close(sockfds[i]);
            }
            break;    
        }
    }while(true);    

    return 0;
}

int init(char** input,int* bloomSize,int* numMonitors,int* bufferSize,int* cyclicBuffer,int* numThreads,char** dirName)
{
    for(int i = 1;i <= 11;i += 2){
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
        else if(!(strcmp(input[i], "-c"))){
            *cyclicBuffer = atoi(input[i+1]);
        }
        else if(!(strcmp(input[i], "-t"))){
            *numThreads = atoi(input[i+1]);
        }
        else{
            std::cout <<"Wrong params"<< std::endl;
            return -1;
        }
    }
    return 0;
}
