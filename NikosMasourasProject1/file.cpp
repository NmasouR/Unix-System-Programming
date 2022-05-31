#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include<cstdlib>


#include "file.h"


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
	year = date.substr(6, 9);
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
			std::cout << "ERROR IN RECORD " << recordnum << std::endl;
			error = 1;
		}
		else if (virus->not_vaccinated_persons->find(citizen[0])) {//inconsistent entry:citizen already in not_vaccinated list for the virus
			std::cout << "ERROR IN RECORD " << recordnum << std::endl;
			error = 1;
		}
		if (citizen[6] == "YES") {
			dateError = reverseDate(citizen[7]);
			if (dateError == -1) {
				std::cout << "Wrong date format" << std::endl;
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
					curlvl = virus->not_vaccinated_persons->getlvl();
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
					std::cout << "ERROR IN RECORD" << recordnum << std::endl;
					error = 1;
				}
			}
		}
		maxlvl = 0;
		citizen[7].clear();
		error = 0;
	}
	records.close();
	std::cout << "Successfully read inputFile" << std::endl;
	return 0;
}

int BloomStatus(std::string citizenID, std::string virusName, virusHead* viruses,int bloomsize)
{
	virusNode* virus;
	int find;
	if (!(virus = viruses->searchVirus(virusName))) {
		std::cout << "can't find virus " << virusName << std::endl;
		return -1;
	}
	find = bloomSearch(virus->bloomptr, citizenID,bloomsize);
	return find;
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

void vaccineStatusForAll(std::string citizenID, virusHead* viruses)
{
	virusNode* temp = viruses->getHead();
	skipnode* curr;
	if (temp == NULL)
		std::cout << "NO VIRUSES AVAILABLE" << std::endl;
	while (temp != NULL) {
		curr = temp->vaccinated_persons->find(citizenID);
		if (curr == NULL)
			std::cout << temp->virusName << " NO" << std::endl;
		else
			std::cout << temp->virusName << " YES " << curr->date << std::endl;
		temp = temp->next;
	}
}

int populationStatusCountry(std::string country,std::string virus, std::string date1, std::string date2, virusHead* viruses,int command)
{
	int persons;
	virusNode* node;
	if((node = viruses->searchVirus(virus)) == NULL)
		return -1;
	if (command == 1)
		persons = node->vaccinated_persons->popCountryStatus(country, date1, date2);//vaccinated in dates range
	else if(command == 0)
		persons = node->not_vaccinated_persons->GetByCountry(country);//not vaccinated
	else if(command == 2)
		persons = node->vaccinated_persons->GetByCountry(country);//vaccinated at any date
	return persons;
}

void populationStatusAllCountries(std::string virus, std::string date1, std::string date2, virusHead* viruses, countryHead* countries)
{
	Country* temp = countries->getHead();
	int vaccinated,notvaccinated,allVaccinated;
	float persent;
	if(date1 == "0" && date2 == "0"){//no dates in input
		while (temp != NULL) {
		allVaccinated = populationStatusCountry(temp->countryName, virus, date1, date2, viruses,2);
		if(allVaccinated == -1){
			std::cout << "Virus " << virus << " doesn't exists" << std::endl;
			return;
		}
		notvaccinated = populationStatusCountry(temp->countryName, virus, date1, date2, viruses,0);
		if ((allVaccinated + notvaccinated) != 0) {
			persent = ((float)allVaccinated / (allVaccinated + notvaccinated)) * 100;
			std::cout << temp->countryName << " " << allVaccinated << " " << persent << "%" << std::endl;
		}
		else
			std::cout << temp->countryName << " " << allVaccinated << " " << 0 << "%" << std::endl;
		temp = temp->next;
		}
		return;
	}
	while (temp != NULL) {
		vaccinated = populationStatusCountry(temp->countryName, virus, date1, date2, viruses,1);
		if(vaccinated == -1){
			std::cout << "Virus " << virus << " doesn't exists" << std::endl;
			return;
		}
		allVaccinated = populationStatusCountry(temp->countryName, virus, date1, date2, viruses,2);
		notvaccinated = populationStatusCountry(temp->countryName, virus, date1, date2, viruses,0);
		if ((allVaccinated + notvaccinated) != 0) {
			persent = ((float)vaccinated / (allVaccinated + notvaccinated)) * 100;
			std::cout << temp->countryName << " " << vaccinated << " " << persent << "%" << std::endl;
		}
		else
			std::cout << temp->countryName << " " << vaccinated << " " << 0 << "%" << std::endl;
		temp = temp->next;
	}
}

int popStatusCountry(std::string country, std::string virus, std::string date1, std::string date2,int age, virusHead* viruses)
{
	int vaccinatedByAge,notVaccinatedByAge,allVaccinatedByAge,smallage;
	float persent;
	virusNode* node;
	if((node = viruses->searchVirus(virus)) == NULL){
		std::cout << "Virus " << virus << " doesn't exists" << std::endl;
		return -1;
	}
	if (age != 121) {
		smallage = age - 20;
		notVaccinatedByAge = node->not_vaccinated_persons->GetByCountryAge(country, smallage, age);
		allVaccinatedByAge = node->vaccinated_persons->GetByCountryAge(country,smallage,age);
		if(date1 == "0" && date2 == "0"){//no date input
			if ((allVaccinatedByAge + notVaccinatedByAge) != 0) {
				persent = ((float)allVaccinatedByAge / (allVaccinatedByAge + notVaccinatedByAge)) * 100;
				std::cout << age - 20 << "-" << age << " " << allVaccinatedByAge << " " << persent << "%" << std::endl;
			}
			else
				std::cout << age - 20 << "-" << age << " " << 0 << " " << 0 << "%" << std::endl;
			return 0;
		}
		vaccinatedByAge = node->vaccinated_persons->popStatus(country, smallage, age,date1, date2);
		if ((allVaccinatedByAge + notVaccinatedByAge) != 0) {
			persent = ((float)vaccinatedByAge / (allVaccinatedByAge + notVaccinatedByAge)) * 100;
			std::cout << age - 20 << "-" << age << " " << vaccinatedByAge << " " << persent << "%" << std::endl;
		}
		else
			std::cout << age - 20 << "-" << age << " " << 0 << " " << 0 << "%" << std::endl;
	} 
	else {
		smallage = age - 61;
		notVaccinatedByAge = node->not_vaccinated_persons->GetByCountryAge(country, smallage, age);
		allVaccinatedByAge = node->vaccinated_persons->GetByCountryAge(country,smallage,age);
		if(date1 == "0" && date2 == "0"){
			if ((allVaccinatedByAge + notVaccinatedByAge) != 0) {//no date input
				persent = ((float)allVaccinatedByAge / (allVaccinatedByAge + notVaccinatedByAge)) * 100;
				std::cout << age - 20 << "-" << age << " " << allVaccinatedByAge << " " << persent << "%" << std::endl;
			}
			else
				std::cout << age - 20 << "-" << age << " " << 0 << " " << 0 << "%" << std::endl;
			return 0;
		}
		vaccinatedByAge = node->vaccinated_persons->popStatus(country, smallage, age, date1, date2);
		if ((allVaccinatedByAge + notVaccinatedByAge) != 0) {
			persent = ((float)vaccinatedByAge / (allVaccinatedByAge + notVaccinatedByAge)) * 100;
			std::cout << smallage << "+ " << vaccinatedByAge << " " << persent << "%" << std::endl;
		}
		else
			std::cout << age << "+ " << 0 << " " << 0 << "%" << std::endl;
	}
	return 0;
}

void popStatus(std::string virus, std::string date1, std::string date2, virusHead* viruses, countryHead* countries)
{
	Country* temp = countries->getHead();
	int error;
	while (temp != NULL) {
		std::cout << temp->countryName << std::endl;
		error = popStatusCountry(temp->countryName, virus, date1, date2, 20, viruses);
		if(error == -1)
			break;
		popStatusCountry(temp->countryName, virus, date1, date2, 40, viruses);
		popStatusCountry(temp->countryName, virus, date1, date2, 60, viruses);
		popStatusCountry(temp->countryName, virus, date1, date2, 121, viruses);
		temp = temp->next;
	}
}


void insertCitizenRecord(std::string* citizenRecord, virusHead* viruses, citizenHead* citizens, countryHead* countries, int bloomsize)
{
	int curlvl,maxlvl = 0,dateError;
	float add = K * P;
	virusNode* virus = viruses->searchVirus(citizenRecord[5]);
	skipnode* skip;
	citizenNode* node;
	Country* country;
	if (virus == NULL) {
		virus = new virusNode(citizenRecord[5], bloomsize);
		viruses->addNode(virus);
	}
	if ((country = countries->search(citizenRecord[3])) == NULL) {
		country = new Country(citizenRecord[3]);
		countries->addNode(country);
	}
		
	if (virus->vaccinated_persons->find(citizenRecord[0]) == NULL && virus->not_vaccinated_persons->find(citizenRecord[0]) == NULL) {//no recrord for this virus
		node = citizens->searchCitizen(citizenRecord[0]);
		if (node != NULL) { //citizen exists already
			if (citizenRecord[6].length() != 0) { // YES
				dateError = reverseDate(citizenRecord[6]);
				if (dateError == -1) {
					std::cout << "Wrong date input" << std::endl;
					return;
				}
				if (citizenRecord[1] != node->firstName || citizenRecord[2] != node->lastName || citizenRecord[3] != node->country->countryName 
					|| atoi(citizenRecord[4].c_str()) != node->age) {
					std::cout << "Wrong citizen credentials" << std::endl;
					return;
				}
				addToBloom(virus->bloomptr, citizenRecord[0], bloomsize);
				skip = new skipnode(citizenRecord[0], citizenRecord[6], node);
				curlvl = virus->vaccinated_persons->getlvl();
				for (int i = 0; i < curlvl + 1; i++) {
					if (rand() % K < add)
						break;
					maxlvl += 1;
					if(maxlvl == MAXLVL)
						break;
				}
				virus->vaccinated_persons->insert(*skip, maxlvl);
			}
			else {//NO
				citizenRecord[6] = "0";
				if (citizenRecord[1] != node->firstName || citizenRecord[2] != node->lastName || citizenRecord[3] != node->country->countryName
					|| atoi(citizenRecord[4].c_str()) != node->age) {
					std::cout << "Wrong citizen credentials" << std::endl;
					return;
				}
				skip = new skipnode(citizenRecord[0], citizenRecord[6], node);
				curlvl = virus->not_vaccinated_persons->getlvl();
				for (int i = 0; i < curlvl + 1; i++) {
					if (rand() % K < add)
						break;
					maxlvl += 1;
					if(maxlvl == MAXLVL)
						break;
				}
				virus->not_vaccinated_persons->insert(*skip, maxlvl);
			}
		}
		else {//New citizen ID
			for (unsigned int i = 0; i < citizenRecord[0].length(); i++) {//check for wrong ID input
				if (citizenRecord[0][i] < '0' || citizenRecord[0][i] > '9') {
					std::cout << "ID must have only numerical characters" << std::endl;
					return;
				}
			}
			node = new citizenNode(citizenRecord[0], citizenRecord[1], citizenRecord[2], country, atoi(citizenRecord[4].c_str()));
			citizens->addNode(node);
			if (citizenRecord[6].length() != 0) { // YES
				dateError = reverseDate(citizenRecord[6]);
				if (dateError == -1) {
					std::cout << "Wrong date input" << std::endl;
					delete node;
					return;
				}
				addToBloom(virus->bloomptr, citizenRecord[0], bloomsize);
				skip = new skipnode(citizenRecord[0], citizenRecord[6], node);
				curlvl = virus->vaccinated_persons->getlvl();
				for (int i = 0; i < curlvl + 1; i++) {
					if (rand() % K < add)
						break;
					maxlvl += 1;
					if(maxlvl == MAXLVL)
						break;
				}
				virus->vaccinated_persons->insert(*skip, maxlvl);
			}
			else {//NO
				citizenRecord[6] = "0";
				skip = new skipnode(citizenRecord[0], citizenRecord[6], node);
				curlvl = virus->not_vaccinated_persons->getlvl();
				for (int i = 0; i < curlvl + 1; i++) {
					if (rand() % K < add)
						break;
					maxlvl += 1;
					if(maxlvl == MAXLVL)
						break;
				}
				virus->not_vaccinated_persons->insert(*skip, maxlvl);
			}
		}
	}
	else if (virus->vaccinated_persons->find(citizenRecord[0]) != NULL) {//Already vaccinated for the virus
		node = citizens->searchCitizen(citizenRecord[0]);
		if (citizenRecord[1] != node->firstName || citizenRecord[2] != node->lastName || citizenRecord[3] != node->country->countryName
			|| atoi(citizenRecord[4].c_str()) != node->age) {
			std::cout << "Wrong citizen credentials" << std::endl;
			return;
		}
		std::cout << "ERROR:CITIZEN " << citizenRecord[0] << " ALREADY VACCINATED ON " << virus->vaccinated_persons->find(citizenRecord[0])->date << std::endl;
	}
	else if (virus->not_vaccinated_persons->find(citizenRecord[0]) != NULL) {//Already in not vaccinated list
		node = citizens->searchCitizen(citizenRecord[0]);
		if (citizenRecord[1] != node->firstName || citizenRecord[2] != node->lastName || citizenRecord[3] != node->country->countryName
			|| atoi(citizenRecord[4].c_str()) != node->age) {
			std::cout << "Wrong citizen credentials" << std::endl;
			return;
		}
		dateError = reverseDate(citizenRecord[6]);
		if (dateError == -1) {
			std::cout << "Wrong date format" << std::endl;
			return;
		}
		virus->not_vaccinated_persons->deleteNode(citizenRecord[0]);//delete from not vaccinated and move to vaccinated
		addToBloom(virus->bloomptr, citizenRecord[0], bloomsize);
		skip = new skipnode(citizenRecord[0], citizenRecord[6], node);
		curlvl = virus->vaccinated_persons->getlvl();
		for (int i = 0; i < curlvl + 1; i++) {
			if (rand() % K < add)
				break;
			maxlvl += 1;
			if(maxlvl == MAXLVL)
				break;
		}
		virus->vaccinated_persons->insert(*skip, maxlvl);
	}

}

void vaccinateNow(std::string* citizenRecord, virusHead* viruses, citizenHead* citizens,countryHead* countries, int bloomsize)
{
	int curlvl, maxlvl = 0,year,month,day;
	char yearptr[10], monthptr[3], dayptr[3];
	time_t now = time(0);
	struct tm* ltm;
	ltm = localtime(&now);
	year = 1900 + ltm->tm_year;
	month = 1 + ltm->tm_mon;
	day = ltm->tm_mday;
	sprintf(yearptr, "%d", year);
	sprintf(monthptr, "%d", month);
	sprintf(dayptr, "%d", day);
	 std::string yearstr = std::string(yearptr), monthstr = std::string(monthptr), daystr = std::string(dayptr), date;
	 date = yearstr + "-" + monthstr + "-" + daystr;//current date
	float add = K * P;
	virusNode* virus = viruses->searchVirus(citizenRecord[5]);
	skipnode* skip;
	citizenNode* node;
	Country* country;
	if (virus == NULL) {
		virus = new virusNode(citizenRecord[5], bloomsize);
		viruses->addNode(virus);
	}
	if ((country = countries->search(citizenRecord[3])) == NULL) {
		country = new Country(citizenRecord[3]);
		countries->addNode(country);
	}
	if (virus->vaccinated_persons->find(citizenRecord[0]) == NULL && virus->not_vaccinated_persons->find(citizenRecord[0]) == NULL) {//no recrord for this virus
		node = citizens->searchCitizen(citizenRecord[0]);
		if (node != NULL) { //citizen exists already
			if (citizenRecord[1] != node->firstName || citizenRecord[2] != node->lastName || citizenRecord[3] != node->country->countryName
				|| atoi(citizenRecord[4].c_str()) != node->age) {
				std::cout << "Wrong citizen credentials" << std::endl;
				return;
			}
			addToBloom(virus->bloomptr, citizenRecord[0], bloomsize);
			skip = new skipnode(citizenRecord[0], date, node);
			curlvl = virus->vaccinated_persons->getlvl();
			for (int i = 0; i < curlvl + 1; i++) {
				if (rand() % K < add)
					break;
				maxlvl += 1;
				if(maxlvl == MAXLVL)
					break;
			}
			virus->vaccinated_persons->insert(*skip, maxlvl);
		}
		else {//New citizen ID
			for (unsigned int i = 0; i < citizenRecord[0].length(); i++) {
				if (citizenRecord[0][i] < '0' || citizenRecord[0][i] > '9') {
					std::cout << "ID must have only numerical characters" << std::endl;
					return;
				}
			}
			node = new citizenNode(citizenRecord[0], citizenRecord[1], citizenRecord[2], country, atoi(citizenRecord[4].c_str()));
			citizens->addNode(node);
			addToBloom(virus->bloomptr, citizenRecord[0], bloomsize);
			skip = new skipnode(citizenRecord[0], date, node);
			curlvl = virus->vaccinated_persons->getlvl();
			for (int i = 0; i < curlvl + 1; i++) {
				if (rand() % K < add)
					break;
				maxlvl += 1;
				if(maxlvl == MAXLVL)
					break;
			}
			virus->vaccinated_persons->insert(*skip, maxlvl);
		}
	}
	else if (virus->vaccinated_persons->find(citizenRecord[0]) != NULL) {//already vaccinated
		node = citizens->searchCitizen(citizenRecord[0]);
		if (citizenRecord[1] != node->firstName || citizenRecord[2] != node->lastName || citizenRecord[3] != node->country->countryName
			|| atoi(citizenRecord[4].c_str()) != node->age) {
			std::cout << "Wrong citizen credentials" << std::endl;
			return;
		}
		std::cout << "ERROR: CITIZEN " << citizenRecord[0] << " ALREADY VACCINATED ON " 
			<< virus->vaccinated_persons->find(citizenRecord[0])->date << std::endl;
	}
	else if (virus->not_vaccinated_persons->find(citizenRecord[0]) != NULL) {//Already in not vaccinated list
		node = citizens->searchCitizen(citizenRecord[0]);
		if (citizenRecord[1] != node->firstName || citizenRecord[2] != node->lastName || citizenRecord[3] != node->country->countryName
			|| atoi(citizenRecord[4].c_str()) != node->age) {
			std::cout << "Wrong citizen credentials" << std::endl;
			return;
		}
		virus->not_vaccinated_persons->deleteNode(citizenRecord[0]);
		addToBloom(virus->bloomptr, citizenRecord[0], bloomsize);
		skip = new skipnode(citizenRecord[0], date, node);
		curlvl = virus->vaccinated_persons->getlvl();
		for (int i = 0; i < curlvl + 1; i++) {
			if (rand() % K < add)
				break;
			maxlvl += 1;
			if(maxlvl == MAXLVL)
				break;
		}
		virus->vaccinated_persons->insert(*skip, maxlvl);
	}
}

void listNonVaccinatedPersons(std::string virus, virusHead* viruses)
{
	virusNode* virusnode = viruses->searchVirus(virus);
	if (virusnode == NULL){
		std::cout << "Virus " << virus << " doesn't exists" << std::endl;
		return;
	}
	virusnode->not_vaccinated_persons->printAll();
}
