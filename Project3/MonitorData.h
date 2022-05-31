#pragma once

#include "virusList.h"
#include "bloom.h"

int inputFromFile(std::string file, virusHead& viruses,citizenHead& citizens,countryHead& countries, int bloomsize);

int reverseDate(std::string& date);

std::string vaccineStatus(std::string citizenID, std::string virusName, virusHead* viruses);

int datecmp(std::string date1,std::string date2);

int fileAdd(virusHead* viruses,citizenHead* citizens,countryHead* countries, int bloomsize,const char* directory);

std::string vaccineCitizenStatus(std::string citizenID, std::string virusName, virusHead* viruses);

void printMonitorStats(countryHead* countries);

int datecmp(std::string date1,std::string date2);