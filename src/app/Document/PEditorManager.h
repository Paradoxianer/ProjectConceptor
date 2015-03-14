#ifndef EDITOR_MANAGER_H
#define EDITOR_MANAGER_H

#include "PEditor.h"
#include "PDocument.h"

#include <app/Message.h>
#include <app/Messenger.h>
#include <support/Archivable.h>
#include <support/List.h>

/**
 * @class PEditorManager
 *
 * @brief  PEditorManager manges all Editors wich are added to a PDocument
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 * @see PDocument
 * @see PEditor
 */
class PEditorManager : public BArchivable
{

public:
							PEditorManager(PDocument *initDoc);
							PEditorManager(BMessage *archive);
	virtual	status_t		Archive(BMessage* archive, bool deep = true) const;
			BArchivable*	Instantiate(BMessage *archive);
	virtual	void			BroadCast(BMessage *message);
	virtual	void			UnregisterPEditor(PEditor *editor);
	virtual	status_t		RegisterPEditor(PEditor *editor);
	virtual PEditor*		GetActivPEditor(void);
	virtual int32			IndexOf(PEditor *editor){return editors->IndexOf(editor);};
	virtual void			SetActivePEditor(int32 index);
	virtual void			SetInactivePEditor(int32 index);
	virtual	PEditor*		InstantiatePEditor(BMessage *archive);
	virtual	int32			CountPEditors(void){return editors->CountItems();};
	virtual	PEditor*		PEditorAt(int32 index){return (PEditor *)editors->ItemAt(index);};

	virtual	PDocument*		BelongTo(void){return doc;};
	
protected:
			void			Init(void);
			void			Init(BMessage *archive);
			
			BList			*editors;
			BList			*editorMessenger;
	static	bool			SendMessage(void *arg,void *msg);
			PDocument		*doc;
			BList			*activeEditor;
private:


};
#endif
