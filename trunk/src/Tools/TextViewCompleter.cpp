#include "TextViewCompleter.h"


TextViewCompleter::TextViewCompleter(bool _match_case=true)
: BMessageFilter(B_PROGRAMMED_DELIVERY,B_ANY_SOURCE,B_KEY_DOWN)
{
		Init();
}

void TextViewCompleter::~TextViewCompleter(void)
{
}

//delmitier stuff
void TextViewCompleter::AddDelimiter(const BString& _delimiter)
{
	fDelimiters.insert(_delimiter);
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
	set<BString>::iterator	iter;
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
	fCompletions.insert(_string);
}

void TextViewCompleter::SetCompletionList(BList *stringList)
{
		// @todo: need a try catch block
	for (int32 i=0;i<stringList->CountItems();i++)
	{
		fCompletions.insert((BString *)stringList->ItemAt(i));
	}
}

BList TextViewCompleter::CompletionList(void)
{
	set<BString>::iterator	iter;
	BList					*returnList	=new BList();
	iter = fCompletions.begin();
	while (iter!=fCompletions.end())
	{
		returnList->AddItem(*iter);
		iter++;
	}
	return returnList;
}

void TextViewCompleter::ClearCompletitionList()
{
		fCompletions.clear();
}



filter_result TextViewCompleter::Filter(BMessage *message, BHandler **target)
{
	set<BString>::iterator	iter;
	bool					startWith 	= false;
	bool					found		= false;
	iter 	= fDelimiters.begin();
	while ( (iter!=fDelimiters.end()) && (!found) )
	{
		found = iter->Cm
	}
	iter 	= fMatches.begin();
	while (iter!=fMatches.end())
	{
		if fCaseSensitive
			startWith = (iter->Compare(fMatch,fMatch.Length()) == 0);
		else
			startWith = (iter->ICompare(fMatch,fMatch.Length()) ==0);
		if (!startWith)
			fMatches.erase(iter);
		startWWith = false;
		iter++;
	}
}


void TextViewCompleter::Init(void)
{
		fDelimiters		= set<BString>();
		fCompletions	= set<BString>();
		fCaseSensitive	= true;
		fMatch			= BString();
		flistView		= new BListView(BRect(0,0,200,400),B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
}

void TextViewCompleter::ShowChoice()
{
	fschowList
}

void TextViewCompleter::NewChoice()
{

	fMatch = BString();
}
