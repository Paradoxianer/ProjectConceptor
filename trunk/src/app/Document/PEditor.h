#ifndef EDITOR_H
#define EDITOR_H

#include "PCommand.h"
#include "PluginManager.h"
#include "ProjectConceptorDefs.h"

#include <app/Handler.h>
#include <app/Message.h>
#include <interface/Rect.h>
#include <interface/Region.h>
#include <interface/View.h>
#include <support/Archivable.h>
#include <support/List.h>
#include "ShortCutFilter.h"


class PEditorManager;
class PDocument;
/**
 * @class PEditor
 *
 * @brief  PEditor is the responsibel to render BMessages and to give the User
 * the posibility to edit the value of the individual BMessage
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@projectconceptor.de
 *
 * Created on: Wed Jun 05  2005
 *
 * @see BMessage
 */

class PEditor : public ObjectHelp, public BArchivable
{

public:
							PEditor(void);
	/**
	 * AttachedToManager is called after The Editor was added to a EditorManager 
	 * at this point you can acces the doc Value
	 */
	virtual	void			AttachedToManager(void)			= 0;
	/**
	 * DetachedFromManager is called after The Editor was removed from the EditorManager 
	 * at this point you can acces the doc Value
	 */
	virtual void			DetachedFromManager(void)		= 0;

	/**
	 * this Method can perform Steps wich are necessary befor saving
	 * e.g. converting Pointer into Messages wich are added to the 
	 * NodeMessage so that you can deal with pointer during the work with Nodes
	 * but if it saved the Data woundt get lost
	 * the Container contains pointer to the all Lists wich are used to store the PDocumentData
	 * @warning the BList wich you are pointet in the Message must or aren´t the same like the one you get over the related PDocument Funktion
	 *<table>
 	 *	<tr><th>Comparable PDcoument List</th><th>BMessage NameField</th><th>Description</th></tr>
	 *	<tr><td>PDocument::GetSelected()</td><td>"Selected"</td></tr>
	 *	<tr><td>PDocument::GetAllNodes()</td><td>"AllNodes"</td></tr>
	 *	<tr><td>PDocument::GetAllConnections()</td><td>"AllConnections"</td></tr>
	 *	<tr><td>PDocument::GetChangedNodes</td><td>"ChangedNodes"</td></tr> 	
	 *</tabale>
	 *
	 *@see PreprocessBeforLoad()
 	 */
	virtual void			PreprocessBeforSave(BMessage *container) = 0;
/**
	 * this Method can perform Steps wich are necessary befor loading
	 * e.g. converting Messages back into valid Objects and add the Objects as Pointer and remove the ObjectMessage
	 *
	 * the Container contains pointer to the all Lists wich are used to store the PDocumentData
	 * @warning the BList wich you are pointet in the Message must or aren´t the same like the one you get over the related PDocument Funktion
	 *<table>
 	 *	<tr><th>Comparable PDcoument List</th><th>BMessage NameField</th><th>Description</th></tr>
	 *	<tr><td>PDocument::GetSelected()</td><td>"Selected"</td></tr>
	 *	<tr><td>PDocument::GetAllNodes()</td><td>"AllNodes"</td></tr>
	 *	<tr><td>PDocument::GetAllConnections()</td><td>"AllConnections"</td></tr>
	 *	<tr><td>PDocument::GetChangedNodes</td><td>"ChangedNodes"</td></tr> 	
	 *</tabale>
	 *
	 *@see PreprocessBeforSave()
 	 */

	virtual void			PreprocessAfterLoad(BMessage *container) = 0;

	/**
	 * if the Editor is an View (this mostly shoud be the case)
	 * it returns the pointer to his View (this shoud be the this pointer :-))
	 */
	virtual BView*			GetView(void) 					= 0;

	/**
	 * if the View wich should be printet is different than the View 
	 * return by GetView you shoudl overwrite this Method and return the
	 * View wich should be printed.
	 */
	virtual BView*			GetPrintView(void) {return NULL;};

	/**
	 * if the Editor is an View it returns the pointer to the view (this pointer)
	 * else it shoud be the pointer to the handler for the Editor (e.g. a Network Client)
	 */
	virtual BHandler*		GetHandler(void) 				= 0;
	/**
	 * Implement this Method and return true if you need a liveupdate if something changed
	 * even if the Editor is not active 
	 *
	 *(at the moment this is ignored and the liveupdate ist sent to)
	 * every registered Editor.. but this will change in future
	 */
	virtual bool			NeedLiveFeed(void){return false;}
	/**
	 * this Method isnt called direcly (at the moment) instead a P_C_VALUE_CHANGED Message is send to the by  GetHandler returned Handler
	 * imlement this that it handle alle changed Nodes wich returned in the doc->GetChangedNodes()
	 * and also in doc->GetTrash() all deleted nodes ;-)
	 * @see GetHandler()
	 */
	virtual	void			ValueChanged(void);
	/**
	 * @brief set the Manager to wich the PEditor  shoud belong  to
	 * calls also the AttachedToManager Method or DetachedFromManager
	 * if the passed Value is NULL
	 * 
	 * @see PEditorManager 
	 */
			void			SetManager(PEditorManager *newManager);
	/**
	 * @brief Returns the Manger to wich the PEditor aktually belongs
	 * 
	 * @see PEditorManager 
	 */
			PEditorManager*	Manager(void){return manager;};

	virtual	BMessage*		GetConfiguration(void) = 0;
	virtual void			SetConfiguration(BMessage *message) = 0;
	
	virtual	void			SetShortCutFilter(ShortCutFilter *_shortCutFilter) = 0;
	
	/**
	 * @brief Returns the Document to wich this editor was added
	 * 
	 * @see PDocument 
	 */
			PDocument		*BelongTo(void){return doc;};

protected:
			void			Init(void);
			/**
			 * gives you a refference to the PEditorManager to wich this editor was added
			 */
			PEditorManager	*manager;
			/**
			 * gives you a refference to the PluginManager so that you can load your onw plugins
			 */
			PluginManager	*pluginManager;
			/**
			 * gives you a refference to the PDocument to wich this editor is added
			 */
			PDocument		*doc;
private:

};
#endif
