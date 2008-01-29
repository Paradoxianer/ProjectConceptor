#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "PEditor.h"
#include "PWindow.h"
#include "PDocument.h"

#include <app/Message.h>
#include <support/Archivable.h>
#include <support/List.h>

/**
 * @class WindowManager
 *
 * @brief  WindowManager manges all Windows wich are added to a PDocument
 *
 * This class is used in the future to manage if one dokument hase multiple Windows.
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 * @todo Implement void WindowManager::TitleWindows(int32 flag)
 * @todo Implement void WindowManager::CascadeWindows(void)
 * @todo Implement PWindow* WindowManager::GetPWindow(uint32 flag)
 * @todo Implement PWindow* WindowManager::GetPWindow(uint32 flag)
 *
 * @see PDocument
 * @see PWindow
 * @see BArchivable
 */

class WindowManager : public BArchivable
{

public:
									WindowManager(PDocument *initDoc);
									WindowManager(BMessage *archive);
									~WindowManager(void);
	static	BArchivable*			Instantiate(BMessage *archive);
	virtual	status_t				Archive(BMessage *archive, bool deep = true) const;
	
	/**
	 *adds a PWindow to the List
	 */
	virtual	void					AddPWindow(PWindow* pWindow);
	/**
	 *removes a PWindow from the List
	 */
	virtual	void					RemovePWindow(PWindow* prc_Window);
	/**
	 *This call titels all Windows in the List the Flag is used to pass the method eg. if vertical horizontal or something else
	 */
	virtual	void					TitleWindows(int32 flag);
	/**
	 *This call cascade all Windows in the List
	 */
	virtual	void					CascadeWindows(void);

	/**
	 *returns the PWindow where this Editor belongs to
	 */
	virtual	PWindow*				GetPWindow(PEditor *editor);
	
	//** dont konw what this was planned for :)
//	virtual	PWindow*				GetPWindow(uint32 flag);
	/**
	 *returns the active PWindow. This means where the user is working in right now
	 */
	virtual	PWindow*				GetActivePWindow(void);
	/**
	 *returns the Number of the PWindows in the List
	 */
	virtual	int32					CountPWindows(void){return windowList->CountItems();};
	/**
	 *returns the PWindow at the given position
	 */
	virtual	PWindow*				PWindowAt(int32 index){return (PWindow *)windowList->ItemAt(index);};
	/**
	 *returns to wich PDocument this WindowManager belongs
	 */
	virtual PDocument*				BelongTo(void){return document;};

protected:
	virtual	void					Init();
			BList*					windowList;
			BRect					windowRect;
			PDocument				*document;
private:

};
#endif
