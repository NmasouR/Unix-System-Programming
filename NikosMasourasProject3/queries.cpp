#include <iostream>
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <fcntl.h> 

#include "queries.h"
#define PERMS 0666

int findCountry(std::string country,travelCountryHead* node){
    travelCountryNode* temp = node->getHead();
    while(temp != 0){
        if(temp->countryName == country){
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

int BloomStatus(std::string citizenID, std::string virusName, travelVirusHead* viruses,int bloomsize)
{
	travelVirusNode* virus;
	int find;
	if (!(virus = viruses->searchVirus(virusName))) {
		std::cout << "can't find virus " << virusName << std::endl;
		return -1;
	}
	find = bloomSearch(virus->bloomptr, citizenID,bloomsize);
	return find;
}

int reverseDate(std::string& date) {
	std::string year, month, day;
	if (date.length() != 10)
		return -1;
	day = date.substr(0, 2);
	if (day > "30" || day < "01")
		return -1;
	month = date.substr(3, 2);
	if (month > "12" || month < "01")
		return -1;
	year = date.substr(6, 4);
	if (year < "1950" || year > "2021")
		return -1;
	date = year + "-" + month + "-" + day;
	return 0;
}

int datecmp(std::string date1,std::string date2){
    std::string year1, month1,day1,year2,month2,day2;
    year1 = date1.substr(0,4);
    month1 = date1.substr(5,2);
    day1 = date1.substr(8,2);
    year2 = date2.substr(0,4);
    month2 = date2.substr(5,2);
    day2 = date2.substr(8,2);
    if(date1 == date2)
        return 1;
    if(year1 < year2)
        return 2;
    else if(year1 == year2){
        if((stoi(month1) - stoi(month2)) <= 6 && (stoi(month1) - stoi(month2)) >= 0){
            if((stoi(month1) - stoi(month2)) > 0)
                return 1;
            else{
                if(stoi(day1) > stoi(day2))
                    return 1;
                else return 2;
            }
        }
        else return 2;
    }
    else if(stoi(year1) == stoi(year2) + 1 ){
            if(stoi(month1) <= 3 && stoi(month2) >= 10)
                return 1;
            else return 2;
    }
    else return 2;
}

int travelRequestGet(char* buf,std::string date1,int n){
    char temp[30];
    int msgstart = 0,err;
    std::string date2;
    for(int i = 0;i < n;i++){
        if(buf[i] == '\0'){
            memcpy(temp,&buf[msgstart],(i - msgstart +1)*sizeof(char));
            msgstart = i + 1;
            if(!strcmp(temp,"NO")){
                return 0;
            }
            else if(!strcmp(temp,"YES")){
                memset(temp,0,sizeof(temp));
            }
            else
                break;
        }
    }
    date2.assign(temp);//reversed
    err = reverseDate(date1);
    if(err < 0){
        std::cout << "wrong date input" << std::endl;
        return -1;
    }
    return datecmp(date1,date2);

}


void statsAllCountries(travelCountryHead** countries,std::string date1,std::string date2,int numsMonitor,std::string virus){
    travelCountryNode* temp;
    int stats[3] = {0},*returned;
    for(int i = 0;i< numsMonitor;i++){
        temp = countries[i]->getHead();
        while(temp != NULL){
            returned = temp->getStats(date1,date2,virus);
            for(int j = 0;j < 3;j++)
                stats[j] += returned[j];
            delete[] returned;
            temp = temp->next;
        }
    }
    std::cout << "TOTAL REQUESTS " << stats[0] << std::endl;
    std::cout << "ACCEPTED " << stats[1] << std::endl;
    std::cout << "REJECTED " << stats[2] << std::endl;
}

int statsCountry(travelCountryHead** countries,std::string country,std::string date1,std::string date2,int numMonitors,std::string virus){
    int found,monitorFound,*stats;
    for(int i = 0;i < numMonitors;i++){
        found = findCountry(country,countries[i]);
        if(found == 1){
            monitorFound = i;
            break;
        }
    }
    if(found == 0){
        std::cout << "Can't find country" << std::endl;
        return -1;
    }
    stats = countries[monitorFound]->search(country)->getStats(date1,date2,virus);
    std::cout << "TOTAL REQUESTS " << stats[0] << std::endl;
    std::cout << "ACCEPTED " << stats[1] << std::endl;
    std::cout << "REJECTED " << stats[2] << std::endl;
    delete[] stats;
    return 0;
}

void GetVaccinationStatus(char* buf,std::string ID,int n,int readfd){
    char temp[30];
    std::string firstName,lastName,age,vaccinated,virus,country;
    int msgstart = 0;
    for(int i = msgstart;i < n;i++){
        if(buf[i] == '\0'){
            memcpy(temp,&buf[msgstart],(i - msgstart +1)*sizeof(char));
            msgstart = i + 1;
            break;
        }
    }
    firstName.assign(temp);
    if(firstName == "end"){
        return;
    }
       
    for(int i = msgstart;i < n;i++){
        if(buf[i] == '\0'){
            memcpy(temp,&buf[msgstart],(i - msgstart +1)*sizeof(char));
            msgstart = i + 1;
            break;
        }
    }
    lastName.assign(temp);
    for(int i = msgstart;i < n;i++){
        if(buf[i] == '\0'){
            memcpy(temp,&buf[msgstart],(i - msgstart +1)*sizeof(char));
            msgstart = i + 1;
            break;
        }
    }
    country.assign(temp);

    for(int i = msgstart;i < n;i++){
        if(buf[i] == '\0'){
            memcpy(temp,&buf[msgstart],(i - msgstart +1)*sizeof(char));
            msgstart = i + 1;
            break;
        }
    }
    age.assign(temp);
    std::cout << ID << " " << firstName << " " << lastName << " " << country << std::endl;
    std::cout << "Age " << age << std::endl;
    memset(temp,0,sizeof(temp));
    while(true){
        for(int i = msgstart;i < n;i++){
            if(buf[i] == '\0'){
                memcpy(temp,&buf[msgstart],(i - msgstart +1)*sizeof(char));
                msgstart = i + 1;
                break;
            }
        }
        virus.assign(temp);
        if(virus == "end")
            break;
        for(int i = msgstart;i < n;i++){
            if(buf[i] == '\0'){
                memcpy(temp,&buf[msgstart],(i - msgstart +1)*sizeof(char));
                msgstart = i + 1;
                break;
            }
        }
        vaccinated.assign(temp);

        if(vaccinated == "NO")
            std::cout << virus << " NOT YET VACCINATED" << std::endl;
        else
            std::cout << virus << " VACCINATED ON " << vaccinated << std::endl;

    }
}

void printStats(travelCountryHead** countries,int numsMonitor){
    travelCountryNode* temp;
    FILE* fp;
    char pid[10];
    char nums[10];
    char fileName[30];
    sprintf(pid,"%d",getpid());
    memcpy(fileName,"log_file.",strlen("log_file.") + 1);
    strcat(fileName,pid);
    fp = fopen(fileName,"w");
    for(int i = 0;i < numsMonitor;i++){
        temp = countries[i]->getHead();
        while(temp != NULL){
            fprintf(fp,"%s",temp->countryName.c_str());
            fprintf(fp,"\n");
            temp = temp->next;
        }
    }
    int stats[3] = {0},*returned;
    for(int i = 0;i< numsMonitor;i++){
        temp = countries[i]->getHead();
        while(temp != NULL){
            returned = temp->getStatsAllDates();
            for(int j = 0;j < 3;j++)
                stats[j] += returned[j];
            delete[] returned;
            temp = temp->next;
        }
    }
    sprintf(nums,"%d",stats[0]);
    fprintf(fp,"TOTAL TRAVEL REQUESTS ");
    fprintf(fp,"%s",nums);
    fprintf(fp,"\n");
    memset(nums,0,sizeof(nums));

    sprintf(nums,"%d",stats[1]);
    fprintf(fp,"ACCEPTED ");
    fprintf(fp,"%s",nums);
    fprintf(fp,"\n");
    memset(nums,0,sizeof(nums));

    sprintf(nums,"%d",stats[2]);
    fprintf(fp,"REJECTED ");
    fprintf(fp,"%s",nums);
    fprintf(fp,"\n");

    fclose(fp);
}