#include "virusList.h"

virusHead::virusHead()
{
	head = NULL;
	tail = NULL;
}

void virusHead::addNode(virusNode* node)
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

virusNode* virusHead::searchVirus(std::string virus) const
{
	virusNode* temp = head;
	while (temp != NULL) {
		if (temp->virusName == virus)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

virusHead::~virusHead()
{
	virusNode* temp = head;
	while (temp != tail) {
		head = head->next;
		delete[] temp->bloomptr;
		delete temp->vaccinated_persons;
		delete temp->not_vaccinated_persons;
		delete temp;
		temp = head;
	}
	delete[] tail->bloomptr;
	delete tail->vaccinated_persons;
	delete tail->not_vaccinated_persons;
	delete tail;
}

virusNode::virusNode(std::string virusName,int bloomsize)
	:virusName(virusName)
{
	bloomptr = new char[bloomsize];
	vaccinated_persons = new skiphead;
	not_vaccinated_persons = new skiphead;
	next = NULL;
}
