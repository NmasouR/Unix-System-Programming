#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include<cstdlib>
#include <string.h>
#include <dirent.h>
#include <unistd.h>


#include "MonitorData.h"


#define P 0.5f
#define K 1000
#define MAXLVL 32 //for skiplist


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

int inputFromFile(std::string file, virusHead& viruses,citizenHead& citizens,countryHead& countries, int bloomsize)
{
	srand((unsigned int)time(NULL));
	std::ifstream records(file);
	virusNode* virus;
	skipnode* skip;
	citizenNode* newcitizen;
	Country* newcountry;
	int recordnum = 0,error = 0, maxlvl = 0,curlvl,dateError;
	float add = K * P;
	if (!records.is_open()) {
		std::cout << "error opening the file" << std::endl;
		return -1;
	}
	std::string line,citizen[8];
	while (std::getline(records, line)) {
		recordnum += 1;
		std::stringstream ss(line);
		for (int i = 0; i < 8; i++) {
			std::getline(ss, citizen[i], ' ');
		}
		if (!(virus = viruses.searchVirus(citizen[5]))) {
			virusNode* newvirus = new virusNode(citizen[5],bloomsize);
			viruses.addNode(newvirus);
			virus = newvirus;
		}
		else if (virus->vaccinated_persons->find(citizen[0])) {//inconsistent entry:citizen already vaccinated for the virus
			//std::cout << "ERROR IN RECORD " << recordnum << std::endl;
			error = 1;
		}
		else if (virus->not_vaccinated_persons->find(citizen[0])) {//inconsistent entry:citizen already in not_vaccinated list for the virus
			//std::cout << "ERROR IN RECORD " << recordnum << std::endl;
			error = 1;
		}
		if (citizen[6] == "YES") {
			dateError = reverseDate(citizen[7]);
			if (dateError == -1) {
				//std::cout << "Wrong date format" << std::endl;
				error = 1;
			}
		}
		if (!(newcountry = countries.search(citizen[3]))) {
			newcountry = new Country(citizen[3]);
			countries.addNode(newcountry);
		}
		if (!error){
			if (!(newcitizen = citizens.searchCitizen(citizen[0]))) {
				newcitizen = new citizenNode(citizen[0], citizen[1], citizen[2], newcountry, atoi(citizen[4].c_str()));
				citizens.addNode(newcitizen);
			}
			if (citizen[6] == "YES") {
				skip = new skipnode(citizen[0], citizen[7], newcitizen);
				curlvl = virus->vaccinated_persons->getlvl();//level of skiplist
				for (int i = 0; i < curlvl + 1; i++) {//generate lvl for insertion
					if (rand() % K < add)
						break;
					maxlvl += 1;
					if(maxlvl == MAXLVL)
						break;
				}
				virus->vaccinated_persons->insert(*skip, maxlvl);
				addToBloom(virus->bloomptr, citizen[0],bloomsize);
			}
			else if (citizen[6] == "NO") {
				if (citizen[7].empty()) {
					skip = new skipnode(citizen[0], "0", newcitizen);
					curlvl = virus->vaccinated_persons->getlvl();
					for (int i = 0; i < curlvl + 1; i++) {
						if (rand() % K < add)
							break;
						maxlvl += 1;
						if(maxlvl == MAXLVL)
							break;
					}
					virus->not_vaccinated_persons->insert(*skip, maxlvl);
				}
				else {//inconsistent entry: date exists with NO
					//std::cout << "ERROR IN RECORD" << recordnum << std::endl;
					error = 1;
				}
			}
		}
		maxlvl = 0;
		citizen[7].clear();
		error = 0;
	}
	records.close();
	//std::cout << "Successfully read inputFile " << file << std::endl;
	return 0;
}

std::string vaccineStatus(std::string citizenID, std::string virusName, virusHead* viruses)
{
	virusNode* virus;
	skipnode* vaccinated;
	std::string date;
	if (!(virus = viruses->searchVirus(virusName))) {
		return "-1";
	}
	vaccinated = virus->vaccinated_persons->find(citizenID);
	if (vaccinated != NULL) {
		return vaccinated->date;
	}
	return "0";//not vaccinated
}

int fileAdd(virusHead* viruses,citizenHead* citizens,countryHead* countries, int bloomsize,const char* directory){
	//for each country dir
	//for each txt
	//check first line if i can find it it's ok else is new
	char fulldir[50],country[30];
	std::string name,ID,virus,tempstr,stringfile,line;
	Country* temp = countries->getHead();
	DIR* dir;
	struct dirent* entry;
	strcpy(fulldir,"./");
	strcat(fulldir,directory);
    strcat(fulldir,"/");
	while(temp != NULL){
		strcpy(country,temp->countryName.c_str());
		char tempName[50];
		strcpy(tempName,fulldir);
		strcat(tempName,country);
    	if((dir = opendir(tempName)) == NULL){
        	perror("open");
        	return -1;
    	}
		stringfile.assign(tempName);
		stringfile += '/';
		while((entry = readdir(dir)) != NULL){
        	name.assign(entry->d_name);
        	if(name == "."|| name == "..")
            	continue;
			if(inputFromFile((stringfile + name),*viruses,*citizens,*countries,bloomsize) == -1){
            	std::cout << "Error opening file" << name << std::endl;
            	return -1;
        	}
    	}
		temp = temp->next;
		closedir(dir);
	}
	return 0;
}

std::string vaccineCitizenStatus(std::string citizenID, std::string virusName, virusHead* viruses)
{
	virusNode* virus;
	skipnode* vaccinated;
	std::string date;
	if (!(virus = viruses->searchVirus(virusName))) {
		return "-1";
	}
	vaccinated = virus->vaccinated_persons->find(citizenID);
	if (vaccinated != NULL) {
		return vaccinated->date;
	}
	return "0";//not vaccinated
}

void printMonitorStats(countryHead* countries){
	Country* temp = countries->getHead();
	int acc = 0;
	int rej = 0;
	int total = 0;
	FILE* fp;
    char pid[10];
	char fileName[30];
	char nums[10];
	while(temp != NULL){
		acc += temp->getAcc();
		rej += temp->getRej();
		temp = temp->next;
	}
	total = acc + rej;
    sprintf(pid,"%d",getpid());
    memcpy(fileName,"log_file.",strlen("log_file.") + 1);
    strcat(fileName,pid);
	fp = fopen(fileName,"w");
	temp = countries->getHead();
	while(temp != NULL){
        fprintf(fp,"%s",temp->countryName.c_str());
        fprintf(fp,"\n");
        temp = temp->next;
	}
	sprintf(nums,"%d",total);
    fprintf(fp,"TOTAL TRAVEL REQUESTS ");
    fprintf(fp,"%s",nums);
    fprintf(fp,"\n");
    memset(nums,0,sizeof(nums));

    sprintf(nums,"%d",acc);
    fprintf(fp,"ACCEPTED ");
    fprintf(fp,"%s",nums);
    fprintf(fp,"\n");
    memset(nums,0,sizeof(nums));

    sprintf(nums,"%d",rej);
    fprintf(fp,"REJECTED ");
    fprintf(fp,"%s",nums);
    fprintf(fp,"\n");

    fclose(fp);

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
        return 0;
    else if(year1 == year2){
        if((stoi(month1) - stoi(month2)) <= 6 && (stoi(month1) - stoi(month2)) >= 0){
            if((stoi(month1) - stoi(month2)) > 0)
                return 1;
            else{
                if(stoi(day1) > stoi(day2))
                    return 1;
                else return 0;
            }
        }
        else return 0;
    }
    else if(stoi(year1) == stoi(year2) + 1 ){
            if(stoi(month1) <= 3 && stoi(month2) >= 10)
                return 1;
            else return 0;
    }
    else return 0;
}

