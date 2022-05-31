#pragma once

#include <string>


struct request{
	int accepted;
	int rejected;
	std::string date;
	std::string virusName;
	request* next;
	request(std::string date,int acc,int rej,std::string virusName);
};

struct travelVirusNode {
	std::string virusName;
	char* bloomptr;
	travelVirusNode* next;
	travelVirusNode(std::string virusName,int bloomsize);
};

class travelVirusHead {
private:
	travelVirusNode* head;
	travelVirusNode* tail;
public:
	travelVirusHead();
	void addNode(travelVirusNode* node);
	travelVirusNode* searchVirus(std::string virus) const;
	travelVirusNode* getHead() const { return head; }
	~travelVirusHead();

};

struct travelCountryNode {
	std::string countryName;
	request* requests;
	travelCountryNode* next;
	travelCountryNode(std::string name);
	~travelCountryNode();
	void addRequest(request* req);
	int* getStats(std::string date1,std::string date2,std::string virus) const;
	int* getStatsAllDates() const;
};

class travelCountryHead {
private:
	travelCountryNode* head;
	travelCountryNode* tail;
public:
	travelCountryHead();
	void addNode(travelCountryNode* node);
	travelCountryNode* search(std::string name) const;
	travelCountryNode* getHead() const { return this->head; }
	~travelCountryHead();
};
