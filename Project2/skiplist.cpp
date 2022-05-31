#include "skiplist.h"
#include <stdio.h>
#include <iostream>

skiphead::skiphead()
	:lvl(0)
{
	this->next = new skipnode("0","0",NULL);
}

void skiphead::insert(skipnode& inserted, int maxlvl)
{
	if (this->find(inserted.id) != NULL)
		return;
	if (this->lvl <= maxlvl) {//create new lvls
		do {
			skipnode* newlvl = new skipnode("0","0",NULL);
			newlvl->down = this->next;
			this->next = newlvl;
			this->lvl += 1;
		} while (this->lvl <= maxlvl);
	}
	skipnode* small, * big, * temp, * enter = &inserted;
	small = this->next;
	for (int i = lvl; i > maxlvl; i--) {
		small = small->down;
	}
	for (int i = maxlvl; i >= 0; i--) {//insert for all lvls
		temp = small;

		while (temp->next != NULL) {
			if (temp->next->id < inserted.id)
				temp = temp->next;
			else break;//temp is the immediately smaller element in the curr level
		}
		big = temp->next;
		small = temp;
		enter->next = big;
		small->next = enter;
		if (i != 0) {//create node for curr lvl - 1
			enter->down = new skipnode(inserted.id,inserted.date,inserted.citizen);
			enter = enter->down;
		}
		small = small->down;
	}
}

skipnode* skiphead::find(std::string idToFind) const
{
	skipnode* small, * temp;
	small = this->next;
	for (int i = lvl; i >= 0; i--) {
		temp = small;
		while (temp->next != NULL) {
			if (temp->next->id > idToFind)
				break;
			temp = temp->next;
		}
		if (temp->id == idToFind)
			return temp;
		small = temp->down;
	}
	return NULL;
}

int skiphead::GetByCountry(std::string country){
	int persons = 0;
	skipnode* temp = this->next;
	for (int i = this->lvl; i > 0; i--) { //lvl 0 contains all citizen of the skiplist
		temp = temp->down;
	}
	temp = temp->next;
	while (temp != NULL) {
		if (temp->citizen->country->countryName == country) {
			persons++;
		}
		temp = temp->next;
	}
	return persons;

}

int skiphead::GetByCountryAge(std::string country,int age1,int age2){
	skipnode* temp = this->next;
	int persons = 0;
	for (int i = this->lvl; i > 0; i--) {
		temp = temp->down;
	}
	temp = temp->next;
	while (temp != NULL) {
		if (temp->citizen->country->countryName == country && temp->citizen->age >= age1 && temp->citizen->age <age2)
			persons++;
		temp = temp->next;
	}
	return persons;

}

int skiphead::popCountryStatus(std::string country, std::string date1, std::string date2) const
{
	int persons = 0;
	skipnode* temp = this->next;
	for (int i = this->lvl; i > 0; i--) { //lvl 0 contains all citizen of the skiplist
		temp = temp->down;
	}
	temp = temp->next;
	while (temp != NULL) {
		if (temp->date >= date1 && temp->date <= date2 && temp->citizen->country->countryName == country) {
			persons++;
		}
		temp = temp->next;
	}
	return persons;
}

int skiphead::popStatus(std::string country, int age1, int age2,std::string date1, std::string date2) const
{
	skipnode* temp = this->next;
	int persons = 0;
	for (int i = this->lvl; i > 0; i--) {
		temp = temp->down;
	}
	temp = temp->next;
	while (temp != NULL) {
		if (temp->date >= date1 && temp->date <= date2 && temp->citizen->country->countryName == country && temp->citizen->age >= age1 && temp->citizen->age <age2)
			persons++;
		temp = temp->next;
	}
	return persons;
}

void skiphead::deleteNode(std::string citizenID)
{
	if (this->find(citizenID) == NULL)
		return;
	skipnode* small, * temp,* del;
	small = this->next;
	for (int i = this->lvl; i >= 0; i--) {
		temp = small;
		while (temp->next != NULL) {
			if (temp->next->id < citizenID)
				temp = temp->next;
			else break;
		}
		if (temp->next == NULL) {//node to delete not in this lvl
			small = temp->down;//start search in next lvl from small
			continue;
		}
		else if (temp->next->id == citizenID) {
			del = temp->next;
			temp->next = del->next;
			delete del;
		}
		small = temp->down;
	}
}

void skiphead::printAll() const
{
	skipnode* temp = this->next;
	for (int i = this->lvl; i > 0; i--) {
		temp = temp->down;
	}
	temp = temp->next;
	while (temp != NULL) {
		std::cout << temp->id << " " << temp->citizen->firstName << " " << temp->citizen->lastName << " " << temp->citizen->country->countryName
			<< " " << temp->citizen->age << std::endl;
		temp = temp->next;
	}
}

skiphead::~skiphead()
{
	skipnode* start = this->next;
	skipnode* del = start->next;
	skipnode* temp;
	for (int i = this->lvl; i >= 0; i--) {
		while (del != NULL) {
			temp = del->next;
			delete del;
			del = temp;
		}
		del = start;
		start = start->down;
		delete del;
		if (start != NULL)
			del = start->next;
	}
}

skipnode::skipnode(std::string id, std::string date, citizenNode* citizen)
	:id(id),date(date)
{
	this->citizen = citizen;
	this->next = NULL;
	this->down = NULL;
}
