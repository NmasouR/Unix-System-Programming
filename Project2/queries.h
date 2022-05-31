#include <string>

#include "travelVirus.h"
#include "bloom.h"

int findCountry(std::string country,travelCountryHead* node);

int BloomStatus(std::string citizenID, std::string virusName, travelVirusHead* viruses,int bloomsize);

int reverseDate(std::string& date);

int travelRequestGet(char* buf,std::string date1,int n);

void statsAllCountries(travelCountryHead** countries,std::string date1,std::string date2,int numsMonitor,std::string virus);

int statsCountry(travelCountryHead** countries,std::string country,std::string date1,std::string date2,int numMonitors,std::string virus);

void GetVaccinationStatus(char* buf,std::string ID,int n,int readfd);

void printStats(travelCountryHead** countries,int numsMonitor);