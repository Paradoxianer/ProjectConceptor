#include <Catalog.h>
#include <stdlib.h>

#include "Ask.h"
#include "InputRequest.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Ask"


Ask::Ask():PCommand()
{
}

static const property_info kAskProperties[] = {
	{ "Ask", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Shows a text-input dialog and stores the answer as a string "
		"under \"variable\" (required) in the macro's value context. "
		"prompt/default (both optional) set the dialog's label/starting "
		"text.", 0, {0},
		{ { { {"variable", B_STRING_TYPE}, {"prompt", B_STRING_TYPE},
			  {"default", B_STRING_TYPE} } } } },
};

const property_info* Ask::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kAskProperties;
}

BMessage* Ask::Do(PDocument *doc, BMessage *settings)
{
	BString	prompt(B_TRANSLATE("Value?"));
	settings->FindString("prompt",&prompt);
	if (prompt.Length() == 0)
		prompt	= B_TRANSLATE("Value?");

	BString		variableName;
	status_t	err	= settings->FindString("variable",&variableName);

	BString	defaultText;
	settings->FindString("default",&defaultText);

	bool	wasSet	= false;
	if (err == B_OK) {
		InputRequest	*inputAlert	= new InputRequest(B_TRANSLATE("Macro input"),
			prompt.String(),defaultText.String(),B_TRANSLATE("OK"),B_TRANSLATE("Cancel"));
		char	*input	= NULL;
		int32	button	= inputAlert->Go(&input);
		if ((button == 0) && (manager->GetValueContext() != NULL)) {
			manager->GetValueContext()->RemoveName(variableName.String());
			manager->GetValueContext()->AddString(variableName.String(),input != NULL ? input : "");
			wasSet	= true;
		}
		free(input);
	}

	settings->RemoveName("Ask::Undo");
	BMessage	undoMessage;
	undoMessage.AddString("variable",variableName);
	undoMessage.AddBool("wasSet",wasSet);
	settings->AddMessage("Ask::Undo",&undoMessage);
	settings	= PCommand::Do(doc,settings);
	doc->SetModified();
	return settings;
}

void Ask::Undo(PDocument *doc,BMessage *undo)
{
	BMessage	undoMessage;
	if (undo->FindMessage("Ask::Undo",&undoMessage) == B_OK) {
		BString	variableName;
		bool	wasSet	= false;
		undoMessage.FindString("variable",&variableName);
		undoMessage.FindBool("wasSet",&wasSet);
		// see the class comment - this removes the variable rather than
		// restoring whatever it held before, if anything did
		if (wasSet && (manager->GetValueContext() != NULL))
			manager->GetValueContext()->RemoveName(variableName.String());
	}
	doc->SetModified();
}



void Ask::AttachedToManager(void)
{
}

void Ask::DetachedFromManager(void)
{
}
