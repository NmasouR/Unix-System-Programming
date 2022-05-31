#include <stdio.h>
#include <string.h>
#include "bloom.h"
#include "hash.h"

void addToBloom(char* bloom,std::string citizenID,int bloomsize)
{
	unsigned long ar[16] = { 0 };
	unsigned char* str = new unsigned char[citizenID.length() + 1];
	strcpy((char*)str, citizenID.c_str());//string to char *
	for (int i = 0; i < 16; i++) {
		ar[i] = hash_i(str, i) % (bloomsize * 8);
	}
	for (int i = 0; i < 16; i++) {
		unsigned long bit = ar[i];
		unsigned long index = bit / 8;
		bloom[index] = bloom[index] | (0x01 << bit % 8); //0x01 = 00000001 binary or 1 decimal
	}
	delete[] str;
}

int bloomSearch(char* bloom, std::string citizenID,int bloomsize)
{
	unsigned long ar[16] = { 0 };
	unsigned char* str = new unsigned char[citizenID.length() + 1];
	strcpy((char*)str, citizenID.c_str());
	for (int i = 0; i < 16; i++) {
		ar[i] = hash_i(str, i) %(bloomsize * 8);
	}
	for (int i = 0; i < 16; i++) {
		if (!(bloom[ar[i] / 8] & (0x01 << ar[i] % 8)))
			return 0;
	}
	delete[] str;
	return 1;
}
