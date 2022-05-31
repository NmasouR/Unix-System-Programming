#pragma once

#include "virusList.h"
#include "citizen.h"

int getdir(char *dirName,virusHead* viruses,citizenHead* citizens,countryHead* countries,int bloomsize,const char *directory);

int sendbloom(int writefd,virusHead* viruses,int bloomsize);

int sendvirus(int writefd,virusHead* viruses);

int sendcountries(int writefd,countryHead* countries);

std::string getString(char *buffer,int* msgstart,int n);