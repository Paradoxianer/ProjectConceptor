#include "Remember.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"


Remember::Remember():PCommand()
{
}

static const property_info kRememberProperties[] = {
	{ "Remember", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Saves the current selection under \"variable\" in the macro's "
		"value context - restore it later with Select node=$<variable>.",
		0, {0},
		{ { { {"variable", B_STRING_TYPE} } } } },
};

const property_info* Remember::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kRememberProperties;
}

BMessage* Remember::Do(PDocument *doc, BMessage *settings)
{
	BString	variableName;
	if ((settings->FindString("variable",&variableName) == B_OK)
			&& (manager->GetValueContext() != NULL)) {
		manager->GetValueContext()->RemoveName(variableName.String());
		BList	*selected	= doc->GetSelected();
		for (int32 i=0; i<selected->CountItems(); i++)
			manager->GetValueContext()->AddPointer(variableName.String(),selected->ItemAt(i));
	}
	settings	= PCommand::Do(doc,settings);
	doc->SetModified();
	return settings;
}

void Remember::Undo(PDocument *doc,BMessage *undo)
{
	// nothing to do :) - see the class comment
	doc->SetModified();
}



void Remember::AttachedToManager(void)
{
}

void Remember::DetachedFromManager(void)
{
}
