#ifndef FIND_WINDOW_H
#define FIND_WINDOW_H

#include "ProjectConceptorDefs.h"
#include "PDocument.h"


#include <private/interface/ColumnListView.h>
#include <app/Message.h>
#include <interface/Window.h>
#include <interface/TabView.h>
#include <interface/TextControl.h>
#include <interface/SplitView.h>

//#include <interface/ColumnTypes.h>

/**
 * @class FindWindow
 * @brief Defines a Window to type in a search phrase 
 *
 * @author Paradoxon
 *
 * 
 * the FindWindow enables life searching
 * The first and standart mode is a Simple TextControl
 * the advanced mode offers you plenty of posibilitys either search for values of attributes
 * or find nodes in a given rectangel and so on.
 *
 * @todo figure out how to optimizie locking so that the document isnt loked and we get all changes
 * if there is an editor "working" on the Document
 * @todo define the complete interface ;-(
 */



class FindWindow : public BWindow
{

public:
							FindWindow(PDocument *tmpDoc);
			void			Init();
			void			AdvancedSearch(bool enabled);
			bool			IsAdvanced(void){return advanced;};
	virtual void			MessageReceived(BMessage *message);
                                        
	virtual	void			Quit(void);
protected:
		/*	void			FindNodes(BMessage *node, BString *string);
			bool			FindInNode(BMessage *node,BString *string);*/
			bool			advanced;
private:
			void			CreateViews();
			BRow			*CreateRow(BMessage *tmpNode);
			BTextControl	*searchText;
			PDocument		*doc;
		};
#endif
