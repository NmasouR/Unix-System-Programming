#pragma once

#include <string>

void addToBloom(char* bloom,std::string citizenID,int bloomsize);
int bloomSearch(char* bloom, std::string citizenID,int bloomsize);
