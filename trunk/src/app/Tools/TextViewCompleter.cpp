#include "TextViewCompleter.h"


TextViewCompleter::TextViewCompleter(bool _match_case)
: BMessageFilter(B_PROGRAMMED_DELIVERY,B_ANY_SOURCE,B_KEY_DOWN)
{
	fCaseSensitive = _match_case;
	Init();
}

TextViewCompleter::~TextViewCompleter(void)
{
}

//delmitier stuff
void TextViewCompleter::AddDelimiter(const BString& _delimiter)
{
	fDelimiters.insert(new BString(_delimiter));
}

void TextViewCompleter::AddDelimiterList(const BList *stringList)
{
	// @todo: need a try catch block
	for (int32 i=0;i<stringList->CountItems();i++)
	{
		fDelimiters.insert((BString *)stringList->ItemAt(i));
	}
}


void TextViewCompleter::SetDelimiterList(const BList *stringList)
{
	fDelimiters.clear();
	AddDelimiterList(stringList);
}

BList* TextViewCompleter::DelimiterList(void)
{
	set<BString*>::iterator	iter;
	BList					*returnList	=new BList();
	iter = fDelimiters.begin();
	while (iter!=fDelimiters.end())
	{
		returnList->AddItem(*iter);
		iter++;
	}
	return returnList;
}

void TextViewCompleter::ClearDelimiterList()
{
		fDelimiters.clear();
}

		//completition stuff
void TextViewCompleter::AddCompletion(const BString& _string)
{
	fCompletions.insert(new BString(_string));
}

void TextViewCompleter::SetCompletionList(const BList *stringList)
{
		// @todo: need a try catch block
	for (int32 i=0;i<stringList->CountItems();i++)
	{
		fCompletions.insert((BString *)stringList->ItemAt(i));
	}
}

BList *TextViewCompleter::CompletionList(void)
{
	set<BString*>::iterator	iter;
	BList					*returnList	=new BList();
	iter = fCompletions.begin();
	while (iter!=fCompletions.end())
	{
		returnList->AddItem((void *)*iter);
		iter++;
	}
	return returnList;
}

void TextViewCompleter::ClearCompletitionList()
{
		fCompletions.clear();
}



filter_result TextViewCompleter::Filter(BMessage *message, BHandler **){
	//iterator used for the delimeterlist and the match list
	set<BString*>::iterator	iter;
	bool					startWith 	= false;
	bool					found		= false;
	BString					key;
	iter 	= fDelimiters.begin();
	if (message->what == B_KEY_DOWN ) {
		message->FindString("bytes",&key);
		while ( (iter!=fDelimiters.end()) && (!found) )
			found = (*iter)->Compare(key);
		if (found)
			NewChoice();
		else{
			iter 	= fMatches.begin();
			while (iter!=fMatches.end()) {
				if (fCaseSensitive)
					startWith = ((*iter)->Compare(fWorkingString,fWorkingString.Length()) == 0);
				else
					startWith = ((*iter)->ICompare(fWorkingString,fWorkingString.Length()) ==0);
				if (!startWith)
					fMatches.erase(iter);
				startWith = false;
				iter++;
			}
		}
		//we always allow to pass the message through
	}
	return B_DISPATCH_MESSAGE;
}


void TextViewCompleter::Init(void)
{
		fDelimiters		= set<BString*>();
		fCompletions	= set<BString*>();
		fMatches		= set<BString*>();
		fCaseSensitive	= true;
		fWorkingString	= BString();
		fListView		= new BListView(BRect(0,0,200,400),"fListView",B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
		fShowList		= NULL;
}

void TextViewCompleter::ShowChoice()
{
	if (fShowList == NULL)
	{
		//**calcualte the Postion
		fShowList	= new BWindow(BRect(100,100,400,400),"choiceList",B_FLOATING_WINDOW,0);
		fListView->ResizeTo(fShowList->Bounds().Width(),fShowList->Bounds().Height());
		fShowList->AddChild(fListView);
	}
	fShowList->Show();
	fListView->MakeEmpty();
	set<BString*>::iterator	iter;
	iter 	= fMatches.begin();
	while (iter!=fMatches.end()) {
		fListView->AddItem(new BStringItem((*iter)->String()));
	}
}

void TextViewCompleter::NewChoice()
{

	fWorkingString.SetTo("");
	fMatches.clear();
	fMatches = fCompletions;
}
