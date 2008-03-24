#ifndef TEXT_VIEW_COMPLETER_H
#define TEXT_VIEW_COMPLETER_H

#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/Window.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <support/List.h>
#include <MessageFilter.h>
#include <String.h>
#if defined(__HAIKU__) && __GNUC__ > 3
#include <set>
using namespace std;
#else
#include <cpp/set.h>
#endif


/**
 * @class TextViewCompleter
 *
 * @brief  show a list with completition sugestions
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2008/01/16
 * @Contact: mail@projectconceptor.de
 *
 *
 */

class TextViewCompleter: public BMessageFilter
{

public:
							TextViewCompleter(bool _match_case=true);
							~TextViewCompleter(void);
			//delmitier stuff
			void			AddDelimiter(const BString& _delimiter);
			void			AddDelimiterList(const BList *stringList);
			void			SetDelimiterList(const BList *stringList);
			BList*			DelimiterList(void);
			void			ClearDelimiterList();

			//completition stuff
			void			AddCompletion(const BString& _string);
			//Add this List to the Completition List
			void			AddCompletionList(const BList *stringList);
			//delete the old CompletitionList and add the new one
			void			SetCompletionList(const BList *stringList);
			BList*			CompletionList(void);
			void			ClearCompletitionList();

			//configure the TextViewCompleter
			void			SetMatchCase(bool _match){fCaseSensitive = _match;};
			bool			MatchesCase(){return fCaseSensitive;};

			//the filter
	virtual	filter_result	Filter(BMessage *message, BHandler **target);

protected:
			void			Init(void);
			void			ShowChoice(void);
			void			NewChoice(void);

private:
			set<BString*>	fDelimiters;
			set<BString*>	fCompletions;
			set<BString*>	fMatches;

			BString			fWorkingString;
			BWindow			*fShowList;
			BListView		*fListView;


			bool			fCaseSensitive;


};
#endif
