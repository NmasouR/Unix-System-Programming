#include "travelVirus.h"


travelVirusHead::travelVirusHead()
{
	head = NULL;
	tail = NULL;
}

void travelVirusHead::addNode(travelVirusNode* node)
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

travelVirusNode* travelVirusHead::searchVirus(std::string virus) const
{
	travelVirusNode* temp = head;
	while (temp != NULL) {
		if (temp->virusName == virus)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

travelVirusHead::~travelVirusHead()
{
	travelVirusNode* temp = head;
	while (temp != tail) {
		head = head->next;
		delete[] temp->bloomptr;
		delete temp;
		temp = head;
	}
	delete[] tail->bloomptr;
	delete tail;
}

travelVirusNode::travelVirusNode(std::string virusName,int bloomsize)
	:virusName(virusName)
{
	bloomptr = new char[bloomsize];
	next = NULL;
}

travelCountryHead::travelCountryHead()
{
	head = NULL;
	tail = NULL;
}

void travelCountryHead::addNode(travelCountryNode* node)
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

travelCountryNode* travelCountryHead::search(std::string name) const
{
	travelCountryNode* temp = head;
	while (temp != NULL) {
		if (temp->countryName == name)
			return temp;
		temp = temp->next;
	}
	return NULL;

}

travelCountryHead::~travelCountryHead()
{
	travelCountryNode* temp = head;
	while (temp != tail) {
		head = head->next;
		delete temp;
		temp = head;
	}
	delete tail;

}

travelCountryNode::travelCountryNode(std::string name)
	:countryName(name)
{
	requests = NULL;
	next = NULL;
}

travelCountryNode::~travelCountryNode(){
	request* temp = this->requests;
	while(temp != NULL){
		this->requests = this->requests->next;
		delete temp;
		temp = requests;
	}
}

void travelCountryNode::addRequest(request* req){
	request* temp = requests;
	if(requests == NULL){
		requests = req;
	}
	else{
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = req;
	}
}


int* travelCountryNode::getStats(std::string date1,std::string date2,std::string virus) const
{
	request* temp = requests;
	int* arr = new int[3];//total,acc,rej
	for(int i = 0;i < 3;i++)
		arr[i] = 0;
	while(temp != NULL){
		if((temp->date >= date1) && (temp->date <= date2) && (temp->virusName == virus)){
			if(temp->accepted == 1)
				arr[1]++;
			else
				arr[2]++;
			arr[0]++;
		}
		temp = temp->next;
	}

	return arr;
}

request::request(std::string date,int acc,int rej,std::string virusName)
	:date(date),accepted(acc),rejected(rej),virusName(virusName)
{
	next = NULL;
}

int* travelCountryNode::getStatsAllDates() const
{
	request* temp = requests;
	int* arr = new int[3];//total,acc,rej
	for(int i = 0;i < 3;i++)
		arr[i] = 0;
	while(temp != NULL){
		if(temp->accepted == 1)
			arr[1]++;
		else
			arr[2]++;
		arr[0]++;
		temp = temp->next;
	}
	return arr;
}
