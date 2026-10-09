#include "ProjectConceptorDefs.h"
#include "Resize.h"


Resize::Resize():PCommand()
{
}

static const property_info kResizeProperties[] = {
	{ "Resize", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Resizes the current selection: right/bottom edges by (dx, dy), "
		"optionally left/top edges by (dleft, dtop).", 0, {0},
		{ { { {"dx", B_FLOAT_TYPE}, {"dy", B_FLOAT_TYPE},
			{P_C_RESIZE_LEFT, B_FLOAT_TYPE}, {P_C_RESIZE_TOP, B_FLOAT_TYPE} } } } },
};

const property_info* Resize::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kResizeProperties;
}


void Resize::Undo(PDocument *doc,BMessage *undo)
{
	set<BMessage*>	*changed	= doc->GetChangedNodes();
	PCommand::Undo(doc,undo);
	BMessage	undoMessage;
	if (undo->FindMessage("Resize::Undo",&undoMessage) != B_OK)
		return;
	BMessage	*node		= NULL;
	BRect		oldFrame;
	for (int32 i = 0; undoMessage.FindPointer("node",i,(void **)&node) == B_OK; i++) {
		if (undoMessage.FindRect("oldFrame",i,&oldFrame) == B_OK) {
			node->ReplaceRect(P_C_NODE_FRAME,oldFrame);
			changed->insert(node);
		}
	}
	doc->SetModified();
}

BMessage* Resize::Do(PDocument *doc, BMessage *settings)
{
	BMessage		undoMessage;
	BList			*selected	= doc->GetSelected();
	set<BMessage*>	*changed	= doc->GetChangedNodes();
	float			dx			= 0;
	float			dy			= 0;
	// optional: dragging a left or top handle moves those edges too
	float			dleft		= 0;
	float			dtop		= 0;
	settings->FindFloat(P_C_RESIZE_LEFT,&dleft);
	settings->FindFloat(P_C_RESIZE_TOP,&dtop);
	if ((settings->FindFloat("dx",&dx) == B_OK) && (settings->FindFloat("dy",&dy) == B_OK)) {
		for (int32 i = 0; i < selected->CountItems(); i++) {
			BMessage	*node		= (BMessage *)selected->ItemAt(i);
			BRect		oldFrame;
			if (node->FindRect(P_C_NODE_FRAME,&oldFrame) != B_OK)
				continue;
			undoMessage.AddRect("oldFrame",oldFrame);
			undoMessage.AddPointer("node",node);
			BRect	newFrame	= oldFrame;
			newFrame.left	+= dleft;
			newFrame.top	+= dtop;
			newFrame.right	+= dx;
			newFrame.bottom	+= dy;
			if ((newFrame.IsValid()) && (newFrame.Width() > 20) && (newFrame.Height() > 20)) {
				node->ReplaceRect(P_C_NODE_FRAME,newFrame);
				changed->insert(node);
			}
		}
	}
	doc->SetModified();
	settings->RemoveName("Resize::Undo");
	settings->AddMessage("Resize::Undo",&undoMessage);
	settings= PCommand::Do(doc,settings);
	return settings;
}



void Resize::AttachedToManager(void)
{
}

void Resize::DetachedFromManager(void)
{
}
