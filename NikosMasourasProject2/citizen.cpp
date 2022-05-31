#include "citizen.h"
#include <iostream>

citizenHead::citizenHead()
{
	head = NULL;
	tail = NULL;
}

void citizenHead::addNode(citizenNode* node)
{
	if (head == NULL) {
		head = node;
		tail = node;
	}
	else {
		tail->next = node;
		tail = node;
	}
}

citizenNode* citizenHead::searchCitizen(std::string citizenID) const
{
	citizenNode* temp = head;
	while (temp != NULL) {
		if (temp->citizenID == citizenID)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

citizenHead::~citizenHead()
{
	citizenNode* temp = head;
	while (temp != tail) {
		head = head->next;
		delete temp;
		temp = head;
	}
	delete tail;

}

countryHead::countryHead()
{
	head = NULL;
	tail = NULL;
}

void countryHead::addNode(Country* node)
{
	if (head == NULL) {
		head = node;
		tail = node;
	}
	else {
		tail->next = node;
		tail = node;
	}
}

Country* countryHead::search(std::string name) const
{
	Country* temp = head;
	while (temp != NULL) {
		if (temp->countryName == name)
			return temp;
		temp = temp->next;
	}
	return NULL;

}

countryHead::~countryHead()
{
	Country* temp = head;
	while (temp != tail) {
		head = head->next;
		delete temp;
		temp = head;
	}
	delete tail;

}
