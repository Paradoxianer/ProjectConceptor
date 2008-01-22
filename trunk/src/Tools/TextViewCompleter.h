#ifndef TEXT_VIEW_COMPLETER_H
#define TEXT_VIEW_COMPLETER_H

#include <app/Message.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <support/List.h>
#include <MessageFilter.h>
#include <String.h>
#include <cpp/set.h>


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
			void			SetDelimiterList(BList *stringList);
			BList*			DelimiterList(void);
			void			ClearDelimiterList();

			//completition stuff
			void			AddCompletion(const BString& _string);
			void			SetCompletionList(BList *stringList);
			BList			CompletionList(void);
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
			set<BString>	fDelimiters;
			set<BString>	fCompletions;

			BString			fMatch;
			BWindow			fschowList;
			BListView		*flistView;			
			
			
			bool			fCaseSensitive;
	

};
#endif
