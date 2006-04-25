#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <app/Message.h>
#include <support/Archivable.h>
#include <support/List.h>

#include "PEditor.h"
#include "PWindow.h"
#include "PDocument.h"

/**
 * @class WindowManager
 *
 * @brief  WindowManager manges all Windows wich are added to a PDocument
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 * @see PDocument
 * @see PWindow
 */

class WindowManager : public BArchivable
{

public:
									WindowManager(PDocument *initDoc);
									WindowManager(BMessage *archive);
									~WindowManager(void);
	static	BArchivable*			Instantiate(BMessage *archive);
	virtual	status_t				Archive(BMessage *archive, bool deep = true) const;
	
	virtual	void					AddPWindow(PWindow* pWindow);
	virtual	void					RemovePWindow(PWindow* prc_Window);
	virtual	void					TitleWindows(int32 flag);
	virtual	void					CascadeWindows(void);


	virtual	PWindow*				GetPWindow(PEditor *editor);
	virtual	PWindow*				GetPWindow(uint32 flag);
	virtual	PWindow*				GetActivePWindow(void);
	virtual	int32					CountPWindows(void){return windowList->CountItems();};
	virtual	PWindow*				PWindowAt(int32 index){return (PWindow *)windowList->ItemAt(index);};

	virtual PDocument*				BelongTo(void){return document;};

protected:
	virtual	void					Init();
			BList*					windowList;
			BRect					windowRect;
			PDocument				*document;
private:

};
#endif
