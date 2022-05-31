#pragma once

#include <string>

#include "virusList.h"
#include "bloom.h"


int inputFromFile(std::string file, virusHead& viruses, citizenHead& citizens, countryHead& countries, int bloomsize);

int BloomStatus(std::string citizenID, std::string virusName, virusHead* viruses,int bloomsize);

std::string vaccineStatus(std::string citizenID, std::string virusName, virusHead* viruses);

void vaccineStatusForAll(std::string citizenID, virusHead* viruses);

int populationStatusCountry(std::string country,std::string virus, std::string date1, std::string date2, virusHead* viruses,int command);

void populationStatusAllCountries(std::string virus, std::string date1, std::string date2, virusHead* viruses, countryHead* countries);

int popStatusCountry(std::string country, std::string virus, std::string date1, std::string date2,int age, virusHead* viruses);

void popStatus(std::string virus, std::string date1, std::string date2, virusHead* viruses, countryHead* countries);

void insertCitizenRecord(std::string* citizenRecord, virusHead* viruses, citizenHead* citizens,countryHead*countries, int bloomsize);

void vaccinateNow(std::string* citizenRecord, virusHead* viruses, citizenHead* citizens, countryHead* countries, int bloomsize);

void listNonVaccinatedPersons(std::string virus, virusHead * viruses);

int reverseDate(std::string& date);