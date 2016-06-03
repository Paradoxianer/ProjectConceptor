#include <interface/Button.h>
#include <interface/Box.h>
#include <interface/GroupLayoutBuilder.h>
#include <interface/GroupLayout.h>
#include <interface/SplitLayoutBuilder.h>
#include <interface/SplitView.h>
#include <interface/StringView.h>
#include <interface/SpaceLayoutItem.h>
#include <Catalog.h>

#include "FindWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FindWindow"

FindWindow::FindWindow(PDocument *tmpDoc):BWindow(BRect(50,50,600,400),B_TRANSLATE("Find"),B_FLOATING_WINDOW_LOOK,B_FLOATING_APP_WINDOW_FEEL,B_AUTO_UPDATE_SIZE_LIMITS)
{
	TRACE();
	doc=tmpDoc;
	CreateViews();
    Init();

}

void FindWindow::Init() {
	BList		*nodeList	= doc->GetAllNodes();
	int32		i			= 0;
}

void FindWindow::CreateViews() {
	TRACE();
    searchText				= new BTextControl("searchText", B_TRANSLATE("Search"),"",new BMessage('find'));
    BBox		*advView	= new BBox("Advanced Search");
    BSplitView	*splitter	= new BSplitView(B_VERTICAL);
   
    searchText->SetModificationMessage(new BMessage('live'));
    advView->SetLabel(B_TRANSLATE("Advanced Search"));
    splitter->SetCollapsible(true);
    splitter->AddChild(advView);
    splitter->AddChild(BGroupLayoutBuilder(B_HORIZONTAL)
						.Add(new BButton("okButton",B_TRANSLATE("OK"), new BMessage('ok')))
						.Add(new BButton("channelButton",B_TRANSLATE("Cancel"), new BMessage('cl')))
				);
	splitter->SetItemCollapsed(0,true);
	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
				.Add(BGroupLayoutBuilder(B_HORIZONTAL)
					.SetInsets(10,10,10,0)
					.Add(searchText)
				)
				.Add(splitter)
			);
/*		BGroupLayoutBuilder(B_VERTICAL)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.SetInsets(10,10,10,10)
			.Add(searchText)
			
		)
			.Add(BSplitLayoutBuilder(B_VERTICAL)
			.Add(advView)
		)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.Add(new BButton("okButton",B_TRANSLATE("OK"), new BMessage('ok')))
			.Add(new BButton("channelButton",B_TRANSLATE("Cancel"), new BMessage('cl')))
		)
	);*/

}


void FindWindow::MessageReceived(BMessage *message)
{
	TRACE();
	switch(message->what)
	{
		case 'ok':
		{
			//**Use a small trick an tell the PDocument that the menu was invoked but with a given searchstring
			BMessage	*searchMessage	= new BMessage(MENU_SEARCH_FIND);
			//searchMessage->AddString("Command::Name","Find");
			searchMessage->AddString("searchString",searchText->Text());
			//searchMessage->AddBool("shadow",true);
			BMessenger	sender(doc);
			sender.SendMessage(searchMessage);
			BWindow::Quit();
		}
		break;
		case 'cl':
		{
			//**restore old selection state
			BWindow::Quit();
		}
		break;
		case 'find':
		{
			//**make shure to to an unshadowed find for the "recording Makros and so on"
		}
		break;
		case 'live': 
		{
			BMessage	*searchMessage	= new BMessage(MENU_SEARCH_FIND);
			//searchMessage->AddString("Command::Name","Find");
			searchMessage->AddString("searchString",searchText->Text());
			searchMessage->AddBool("shadow",true);
			BMessenger	sender(doc);
			sender.SendMessage(searchMessage);
		}
		break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

/*
void FindWindow::FindNodes(BMessage *node,BString *search)
{
	char		*attribName		= NULL;
	BMessage	*attribMessage	= new BMessage();
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;
	bool		found			= false;
	for (int32 i = 0; node->GetInfo(B_MESSAGE_TYPE, i,(char **) &attribName, &type, &count) == B_OK; i++)
	{
		if (node->FindMessage(attribName,count-1,attribMessage) == B_OK)
			found = found | FindInNode(attribMessage, search);
	}
}

bool FindWindow::FindInNode(BMessage *node,BString *search)
{
	char		*attribName		= NULL;
	BMessage	*attribMessage	= new BMessage();
	BString		*dataString		= NULL;	
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;
	bool		found			= false;
	int32		i				= 0;
	//first iterate through all Strings
	while ((node->GetInfo(B_STRING_TYPE, i,(char **) &attribName, &type, &count) == B_OK) && !found)
	{
		if ((node->FindString(attribName,count,dataString)==B_OK) && (dataString != NULL))
		{
			found = dataString->FindFirst(*search)!=B_ERROR;
		}
		i++;
	}
	//check all subnodes / sub bmessages
	i=0;
	while ((node->GetInfo(B_MESSAGE_TYPE, i,(char **) &attribName, &type, &count) == B_OK) && !found)
	{
		if ((node->FindMessage(attribName,count,attribMessage) == B_OK) && (attribMessage != NULL))
			found = FindInNode(attribMessage, search);
		i++;
	}
	return found;
}*/


void FindWindow::Quit()
{
	TRACE();
	BWindow::Quit();
}
