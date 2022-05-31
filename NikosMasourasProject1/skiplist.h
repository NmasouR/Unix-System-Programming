#pragma once
#include <iostream>
#include "citizen.h"

struct  skipnode {
	std::string id;
	std::string date;
	citizenNode* citizen;
	skipnode* next;
	skipnode* down;
	skipnode(std::string id, std::string date,citizenNode *citizen);
};

class skiphead {
private:
	skipnode* next;
	int lvl;
public:
	skiphead();
	void insert(skipnode& inserted, int maxlvl);
	skipnode* find(std::string idToFind) const;
	int getlvl() const { return lvl; }
	int GetByCountry(std::string country);
	int GetByCountryAge(std::string country,int age1,int age2);
	int popCountryStatus(std::string country, std::string date1="0", std::string date2="0") const;
	int popStatus(std::string country, int age1,int age2, std::string date1="0", std::string date2="0") const;
	void deleteNode(std::string citizenID);
	void printAll() const;
	~skiphead();
};

