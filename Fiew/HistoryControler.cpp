/*
HistoryControler.cpp
This object controls the undo and redo processing for a particular Workspace.
It stores, adds and removes HistoryElements.
*/

#include "stdafx.h"
#include "Core.h"

HistoryControler::HistoryControler(Workspace *wspc)
{
	this->wspc = wspc;
	this->historyElems = new List<HistoryElement>();
	this->historyElems->add(new HistoryBlankElement());
}

HistoryControler::~HistoryControler()
{
	delete this->historyElems;
}

void HistoryControler::add(HistoryElement *element)
{
	if( this->historyElems->isThatTail() == false ){
		while( this->historyElems->getThat() != this->historyElems->getTail() ){
			delete this->historyElems->removeTail();
		}
	}
	this->historyElems->add(element);
	this->historyElems->gotoTail();

	Core::self->getToolws()->getToolwBoxHistory()->load();
}

void HistoryControler::remove()
{
	if( this->historyElems->isThatHead() == false && this->historyElems->isThatTail() == true ){
		this->undo(false);
		delete this->historyElems->removeTail();
	}
	while( this->historyElems->getThat() != this->historyElems->getTail() ){
		delete this->historyElems->removeTail();
	}
	this->wspc->updateToolws();
	this->wspc->update();
}

void HistoryControler::gotoElement(HistoryElement *element)
{
	int tix = this->historyElems->countLeftoThat();
	int eix = this->historyElems->getIndex(element);

	if( eix <= tix )
		this->undo(element);
	else if( eix > tix )
		this->redo(element);
}

bool HistoryControler::undo(HistoryElement *element)
{
	bool result = false;
	while( this->historyElems->getThat() != element )
		result = this->undo(false);

	return result;
}

bool HistoryControler::redo(HistoryElement *element)
{
	bool result = false;
	while( this->historyElems->getThat() != element )
		result = this->redo(false);

	return result;
}

bool HistoryControler::undo(bool forceUpdate)
{
	bool update = this->historyElems->getThat()->undo();
	bool result = this->historyElems->prev();

	if( update == true && forceUpdate == true ){
		this->wspc->updateToolws();
		this->wspc->update();
	}
	return result;
}

bool HistoryControler::redo(bool forceUpdate)
{
	bool result = this->historyElems->next();
	bool update = this->historyElems->getThat()->redo();

	if( update == true && forceUpdate == true ){
		this->wspc->updateToolws();
		this->wspc->update();
	}
	return result;
}

List<HistoryElement> *HistoryControler::getHistoryElems()
{
	return this->historyElems;
}