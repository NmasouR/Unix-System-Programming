#pragma once

#include <string>

struct Country {
	std::string countryName;
	Country* next;
	int accepted;
	int rejected;
	Country(std::string country)
		:countryName(country) {
		next = NULL;
		accepted = 0;
		rejected = 0;
	}
	int getAcc() const{return accepted;}
	int getRej() const{return rejected;}
};

class countryHead {
private:
	Country* head;
	Country* tail;
public:
	countryHead();
	void addNode(Country* node);
	Country* search(std::string name) const;
	Country* getHead() const { return this->head; }
	~countryHead();
};

struct citizenNode {
	std::string citizenID;
	std::string firstName;
	std::string lastName;
	Country* country;
	int age;
	citizenNode* next;
	
	citizenNode(std::string citizenID, std::string firstName, std::string lastName,Country* country, int age)
		:citizenID(citizenID), firstName(firstName), lastName(lastName), age(age)
	{
		this->country = country;
		next = NULL;
	}
};

class citizenHead {
private:
	citizenNode* head;
	citizenNode* tail;
public:
	citizenHead();
	void addNode(citizenNode* node);
	citizenNode* searchCitizen(std::string citizenID) const;
	~citizenHead();
};

