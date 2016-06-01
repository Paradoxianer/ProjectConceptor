#include <Catalog.h>

#include "HelpManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "HelpWindow"

HelpManager::HelpManager(void): BWindow(BRect(100,100,400,200), B_TRANSLATE("Help"), B_DOCUMENT_WINDOW,0)
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

