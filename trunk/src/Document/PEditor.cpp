#include "PEditor.h"
#include "PDocument.h"
#include "PDocumentManager.h"


#include "PEditorManager.h"

PEditor::PEditor(void)
{
	TRACE();
	Init();
}
void PEditor::Init(void)
{
	doc				= NULL;
	pluginManager	= NULL;
	manager			= NULL;
}

/**
 * Called if a value, wich this editor is, waching was changed
 */

void PEditor::ValueChanged(void)
{
	TRACE();

}

/**
 * Set the EditorManager this Method ist called if the Editor is added or removed from the EditorManager
 */
void PEditor::SetManager(PEditorManager *newManager)
{
	TRACE();
	if (newManager!=NULL)
	{
		manager			= newManager;
		doc				= newManager->BelongTo();
		pluginManager	= (doc->BelongTo())->GetPluginManager();
		AttachedToManager();
	}
	else
	{
		DetachedFromManager();
	}
}
