#include "HelpManager.h"

HelpManager::HelpManager(void): BWindow(BRect(100,100,400,200), _T("Help"), B_DOCUMENT_WINDOW,0)
{
	TRACE();
//	helpObjects	= new map<char*,ObjectHelp*>();
}

void HelpManager::Init(void)
{
}

void HelpManager::Search(void)
{
}

void HelpManager::RegisterHelpObject(ObjectHelp *object)
{
	TRACE();
	helpObjects[object->GetID()] = object;
}


void HelpManager::MessageReceived(BMessage *message)
{
	TRACE();
	switch (message->what)
	{
	}
}

