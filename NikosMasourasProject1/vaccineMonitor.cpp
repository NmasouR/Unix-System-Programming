#include <iostream>
#include <string>
#include <sstream>
#include <string.h>
#include<cstdlib>


#include "virusList.h"
#include "hash.h"
#include "bloom.h"
#include "file.h"
#include "citizen.h"

int main(int argc, char* argv[]) {
	int bloomsize; //in bytes
	int error;
	std::string fileName;
	virusHead* viruses = new virusHead;
	citizenHead* citizens = new citizenHead;
	countryHead* countries = new countryHead;
	if (!(strcmp(argv[1], "-c") && strcmp(argv[3], "-b"))) { //strcmp returns 0 if str1 = str2
		fileName.assign(argv[2]); //char * to string
		bloomsize = atoi(argv[4]);
		if ((error = inputFromFile(fileName, *viruses, *citizens, *countries, bloomsize)) == -1) {
			std::cout << "Error opening file" << std::endl;
			delete viruses;
			delete citizens;
			delete countries;
			return -1;
		}
	}
	else if (!(strcmp(argv[1], "-b") && strcmp(argv[3], "-c"))) {
		fileName.assign(argv[4]);
		bloomsize = atoi(argv[2]);
		if ((error = inputFromFile(fileName, *viruses, *citizens, *countries, bloomsize)) == -1) {
			std::cout << "Error opening file" << std::endl;
			delete viruses;
			delete citizens;
			delete countries;
			return -1;
		}
	}
	else {
		std::cout << "Wrong params" << std::endl;
		delete viruses;
		delete citizens;
		delete countries;
		return -1;
	}
	std::string input;
	std::string command,exit = "/exit";
	int answer;
	do{
		std::getline(std::cin, input);
		std::stringstream ss(input);
		std::getline(ss, command, ' ');
		if ( command == "/vaccineStatusBloom") {
			std::string ID,virus;
			std::getline(ss, ID, ' ');
			std::getline(ss, virus);
			answer = BloomStatus(ID, virus, viruses,bloomsize);
			switch (answer)
			{
			case(0):
				std::cout << "NOT VACCINATED" << std::endl;
				break;
			case(1):
				std::cout << "MAYBE" << std::endl;
				break;
			default:
				std::cout << "ERROR FINDING VIRUS" << std::endl;
				break;
			}
			continue;
		}
		else if (command == "/vaccineStatus") {
			std::string ID, virus,date,substr;
			std::getline(ss, substr);
			if (substr.find_first_of(' ') == std::string::npos) { //has no spaces so is command vaccine status for all viruses
				ID = substr;
				vaccineStatusForAll(ID, viruses);
				continue;
			}
			std::stringstream ss1(substr);
			std::getline(ss1, ID, ' ');
			std::getline(ss1, virus);
			date = vaccineStatus(ID, virus, viruses);
			if (date == "-1")
				std::cout << "ERROR FINDING VIRUS" << std::endl;
			else if (date == "0")
				std::cout << "NOT VACCINATED" << std::endl;
			else
				std::cout << "VACCINATED ON " << date << std::endl;
			continue;
		}
		else if (command == "/populationStatus") { 
			std::string country, date1, date2,virus;
			int vaccinated,dateError,notvaccinated,allVaccinated;
			float persent;
			std::getline(ss, country, ' ');
			Country* countryExists = countries->search(country);
			if (countryExists != NULL) { //country exists
				std::getline(ss, virus, ' ');
				std::getline(ss, date1, ' ');
				if(date1.length() == 0)//no dates input
				{
					allVaccinated = populationStatusCountry(country, virus, date2, date1, viruses, 2);
					if(allVaccinated == -1){
						std::cout << "Virus " << virus << " doesn't exists" << std::endl;
						continue;
					}
					notvaccinated = populationStatusCountry(country, virus, date2, date1, viruses,0);
					if ((allVaccinated + notvaccinated) != 0) {//0/0 NaN
						persent = ((float)allVaccinated / (allVaccinated + notvaccinated)) * 100;
						std::cout << country << " " << allVaccinated << " " << persent << "%" << std::endl;
					}
					else
						std::cout << country << " " << allVaccinated << " " << 0 << "%" << std::endl;
					continue;

				}
				std::getline(ss, date2, ' ');
				if (date2.length() == 0) {//No date2 exists while date1 exists
					std::cout << "ERROR" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date1)) == -1) {//Make format yyyy-mm-dd
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date2)) == -1) {
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if (date1 > date2) {
					notvaccinated = populationStatusCountry(country, virus, date2, date1, viruses, 0);
					if(notvaccinated == -1){
						std::cout << "Virus " << virus << " doesn't exists" << std::endl;
						continue;
					}
					allVaccinated = populationStatusCountry(country, virus, date2, date1, viruses, 2);
					vaccinated = populationStatusCountry(country, virus, date2, date1, viruses,1);
					if ((allVaccinated + notvaccinated) != 0) {//0/0 NaN
						persent = ((float)vaccinated / (allVaccinated + notvaccinated)) * 100;
						std::cout << country << " " << vaccinated << " " << persent << "%" << std::endl;
					}
					else
						std::cout << country << " " << vaccinated << " " << 0 << "%" << std::endl;
				}
				else {
					notvaccinated = populationStatusCountry(country, virus, date1, date2, viruses, 0);
					if(notvaccinated == -1){
						std::cout << "Virus " << virus << " doesn't exists" << std::endl;
						continue;
					}
					allVaccinated = populationStatusCountry(country, virus, date2, date1, viruses, 2);
					vaccinated = populationStatusCountry(country, virus, date1, date2, viruses,1);
					if ((allVaccinated + notvaccinated) != 0) {
						persent = ((float)vaccinated / (allVaccinated + notvaccinated)) * 100;
						std::cout << country << " " << vaccinated << " " << persent << "%" << std::endl;
					}
					else
						std::cout << country << " " << vaccinated << " " << 0 << "%" << std::endl;

				}
			}
			else { //country doesn't exists
				virus = country;
				std::getline(ss, date1, ' ');
				if(date1.length() == 0){
					populationStatusAllCountries(virus, "0", "0", viruses, countries);
					continue;
				}
				std::getline(ss, date2, ' ');
				if (date2.length() == 0) {
					std::cout << "ERROR" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date1)) == -1) {
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date2)) == -1) {
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if (date1 > date2)
					populationStatusAllCountries(virus, date2, date1, viruses, countries);
				else
					populationStatusAllCountries(virus, date1, date2, viruses, countries);
				continue;
			}
		}
		else if (command == "/popStatusByAge") {
			std::string country, virus, date1, date2;
			int dateError,virusErr;
			std::getline(ss, country, ' ');
			Country* countryexists = countries->search(country);
			if (countryexists != NULL) {
				std::getline(ss, virus, ' ');
				std::getline(ss, date1, ' ');
				if(date1.length() == 0){//no date input
					std::cout << country << std::endl;
					virusErr = popStatusCountry(country, virus, "0", "0", 20, viruses);
					if(virusErr == -1)
						continue;
					popStatusCountry(country, virus, "0", "0", 40, viruses);
					popStatusCountry(country, virus, "0", "0", 60, viruses);
					popStatusCountry(country, virus, "0", "0", 121, viruses);
					continue;
				}
				std::getline(ss, date2, ' ');
				if (date2.length() == 0) {//not received date2
					std::cout << "ERROR" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date1)) == -1) {
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date2)) == -1) {
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if (date2 < date1) {
					std::cout << country << std::endl;
					virusErr = popStatusCountry(country, virus, date2, date1, 20, viruses);
					if(virusErr == -1)
						continue;
					popStatusCountry(country, virus, date2, date1, 40, viruses);
					popStatusCountry(country, virus, date2, date1, 60, viruses);
					popStatusCountry(country, virus, date2, date1, 121, viruses);
				}
				else {
					std::cout << country << std::endl;
					virusErr = popStatusCountry(country, virus, date1, date2, 20, viruses);
					if(virusErr == -1)
						continue;
					popStatusCountry(country, virus, date1, date2, 40, viruses);
					popStatusCountry(country, virus, date1, date2, 60, viruses);
					popStatusCountry(country, virus, date1, date2, 121, viruses);

				}
			}
			else {//No country
				virus = country;
				std::getline(ss, date1, ' ');
				if(date1.length() == 0){//no date input
					popStatus(virus, "0", "0", viruses, countries);
					continue;
				}
				std::getline(ss, date2, ' ');
				if (date2.length() == 0) {//No date2
					std::cout << "ERROR" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date1)) == -1) {
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if ((dateError = reverseDate(date2)) == -1) {
					std::cout << "Wrong date input" << std::endl;
					continue;
				}
				if (date2 < date1) {
					popStatus(virus, date2, date1, viruses, countries);
				}
				else
					popStatus(virus, date1, date2, viruses, countries);
				continue;
			}
		}
		else if (command == "/insertCitizenRecord") {
			std::string citizenRecord[7],vaccinated,err;
			std::getline(ss, citizenRecord[0], ' ');
			std::getline(ss, citizenRecord[1], ' ');
			std::getline(ss, citizenRecord[2], ' ');
			std::getline(ss, citizenRecord[3], ' ');
			std::getline(ss, citizenRecord[4], ' ');
			std::getline(ss, citizenRecord[5], ' ');
			std::getline(ss, vaccinated, ' ');
			if (vaccinated.find("YES") != std::string::npos){//is vaccinated
				std::getline(ss, citizenRecord[6]);//date
				if(citizenRecord[6].length() == 0){
					std::cout << "ERROR" << std::endl;
					continue;
				}
			} 
			else if(vaccinated.find("NO")!= std::string::npos){
				std::getline(ss,err);
				if(err.length() != 0){
					std::cout << "ERROR" << std::endl;
					continue;
				}
			}
			insertCitizenRecord(citizenRecord, viruses, citizens,countries, bloomsize);
		}
		else if (command == "/vaccinateNow") {
			std::string citizenRecord[6];
			std::getline(ss, citizenRecord[0], ' ');
			std::getline(ss, citizenRecord[1], ' ');
			std::getline(ss, citizenRecord[2], ' ');
			std::getline(ss, citizenRecord[3], ' ');
			std::getline(ss, citizenRecord[4], ' ');
			std::getline(ss, citizenRecord[5]);
			vaccinateNow(citizenRecord, viruses, citizens, countries, bloomsize);
		}
		else if (command == "/list-nonVaccinated-Persons") {
			std::string virus;
			std::getline(ss, virus);
			listNonVaccinatedPersons(virus, viruses);
		}
		else if (command.find(exit) != std::string::npos) {
			delete countries;
			delete viruses;
			delete citizens;
			break;
		}
	} while (true);
	return 0;
}