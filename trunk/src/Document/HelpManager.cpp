#include "HelpManager.h"

HelpManager::HelpManager(void): BWindow(BRect(100,100,400,200), _T("Help"), B_DOCUMENT_WINDOW,0)
{
	TRACE();
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
}


void HelpManager::MessageReceived(BMessage *message)
{
	TRACE();
}

