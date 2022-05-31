#pragma once

#include <string>
#include "skiplist.h"

struct virusNode {
	std::string virusName;
	char* bloomptr;
	skiphead* vaccinated_persons;
	skiphead* not_vaccinated_persons;
	virusNode* next;
	virusNode(std::string virusName,int bloomsize);
};

class virusHead {
private:
	virusNode* head;
	virusNode* tail;
public:
	virusHead();
	void addNode(virusNode* node);
	virusNode* searchVirus(std::string virus) const;
	virusNode* getHead() const { return head; }
	~virusHead();

};

