#include "Layout.h"

#include <string.h>
#include <support/String.h>

#include "DotLayouter.h"
#include "LayoutCommandBuilder.h"
#include "ProjectConceptorDefs.h"


Layout::Layout():PCommand()
{
}

void Layout::AttachedToManager(void) {
}

void Layout::DetachedFromManager(void) {
}


BMessage* Layout::Do(PDocument *doc, BMessage *settings)
{
	DotLayouter	layouter;
	const char	*direction	= NULL;
	const char	*engine		= NULL;
	if (settings->FindString("direction",&direction) == B_OK)
		layouter.SetRankDir(direction);
	if (settings->FindString("engine",&engine) == B_OK)
		layouter.SetEngine(engine);

	BList	*nodes			= doc->GetAllNodes();
	BList	*connections	= doc->GetAllConnections();
	if ((nodes == NULL) || (nodes->CountItems() == 0))
		return settings;

	if (!layouter.IsAvailable()) {
		BString	text;
		text.SetToFormat("%s is not available - is it installed and on PATH?",
			layouter.Name());
		settings->AddString("error",text.String());
		return settings;
	}

	BMessage	positions;
	status_t	err	= layouter.Layout(nodes,connections,&positions);
	if (err != B_OK) {
		BString	text;
		text.SetToFormat("%s failed: %s",layouter.Name(),strerror(err));
		settings->AddString("error",text.String());
		return settings;
	}

	LayoutCenterOnOldBounds(nodes,&positions);
	LayoutAppendSubCommands(&positions,settings);

	return PCommand::Do(doc,settings);
}


static const property_info kLayoutProperties[] = {
	{ "Layout", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Runs automatic layout on the whole graph.", 0, {0},
		{ { { {"direction", B_STRING_TYPE}, {"engine", B_STRING_TYPE} } } } },
};

const property_info* Layout::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kLayoutProperties;
}
