#include <iostream>
#include <string>
#include <string.h>
#include <dirent.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <unistd.h>


#include "MonitorGet.h"
#include "MonitorData.h"

int getdir(char *dirName,virusHead* viruses,citizenHead* citizens,countryHead* countries,int bloomsize,const char* directory)
{
    DIR* dir;
    std::string name,stringfile;
    struct dirent* entry;
    char fulldir[50];
    char file[50];
    strcpy(fulldir,"./");
    strcat(fulldir,directory);
    strcat(fulldir,"/");
    strcat(fulldir,dirName);
    strcpy(file,fulldir);
    strcat(file,"/");
    stringfile.assign(file);
    if((dir = opendir(fulldir)) == NULL){
        perror("open");
        return -1;
    }
    while((entry = readdir(dir)) != NULL){
        name.assign(entry->d_name);
        if(name == "."|| name == "..")
            continue;
        if(inputFromFile((stringfile + name),*viruses,*citizens,*countries,bloomsize) == -1){
            std::cout << "Error opening file" << name << std::endl;
            return -1;
        }
    }
    closedir(dir);
    return 0;
}

int sendvirus(int writefd,virusHead* viruses){
    virusNode* temp = viruses->getHead();
    int namelen;
    while(temp != NULL){
        namelen = strlen(temp->virusName.c_str()) + 1;
        if(write(writefd,temp->virusName.c_str(),namelen) < 0){
            perror("write");
            return -1;
        }
        temp = temp->next;
    }
    if(write(writefd,"end",(strlen("end") + 1)) != 4){
        std::cout << "couldn't write to monitor" << std::endl;
        return -1;
    }
    return 0;
}

int sendbloom(int writefd,virusHead* viruses,int bloomsize){
    virusNode* temp = viruses->getHead();
    int n;
    while(temp != NULL){
        if((n = write(writefd,temp->bloomptr,bloomsize * sizeof(char))) < 0){
            perror("write ");
            return -1;
        }
        temp = temp->next;
    }
    return 0;
}

int sendcountries(int writefd,countryHead* countries){
    Country* temp = countries->getHead();
    while(temp != NULL){
        if(write(writefd,temp->countryName.c_str(),strlen(temp->countryName.c_str()) + 1) < 0){
            perror("write country");
            return -1;
        }
        temp = temp->next;
    }
    if(write(writefd,"end",(strlen("end") + 1)) != 4){
        std::cout << "couldn't write to monitor" << std::endl;
        return -1;
    }
    return 0;

}

std::string getString(char *buffer,int* msgstart,int n){
    char temp[30];
    std::string tempstr;
    for(int i = *msgstart;i < n;i++){
        if(buffer[i] == '\0'){
            memcpy(temp,&buffer[*msgstart],(i - *msgstart +1)*sizeof(char));
            *msgstart = i + 1;
            break;
        }
    }
    if(*msgstart == n)
        *msgstart = 0;
    return tempstr.assign(temp);
}