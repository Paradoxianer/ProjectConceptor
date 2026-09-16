#include "LayoutCommandBuilder.h"

#include "ProjectConceptorDefs.h"


void LayoutCenterOnOldBounds(const BList *nodes, BMessage *positions)
{
	BRect	oldBounds;
	bool	haveOld		= false;
	for (int32 i = 0; i < nodes->CountItems(); i++) {
		BMessage	*node	= (BMessage*)nodes->ItemAt(i);
		BRect		nodeFrame;
		if ((node != NULL) && (node->FindRect(P_C_NODE_FRAME,&nodeFrame) == B_OK)) {
			oldBounds	= haveOld ? (oldBounds | nodeFrame) : nodeFrame;
			haveOld		= true;
		}
	}

	BRect	newBounds;
	bool	haveNew		= false;
	int32	i			= 0;
	BRect	positionFrame;
	while (positions->FindRect("frame",i,&positionFrame) == B_OK) {
		newBounds	= haveNew ? (newBounds | positionFrame) : positionFrame;
		haveNew		= true;
		i++;
	}

	if ((!haveOld) || (!haveNew))
		return;

	BPoint	oldCenter((oldBounds.left+oldBounds.right)/2,(oldBounds.top+oldBounds.bottom)/2);
	BPoint	newCenter((newBounds.left+newBounds.right)/2,(newBounds.top+newBounds.bottom)/2);
	BPoint	delta	= oldCenter-newCenter;
	if (delta == BPoint(0,0))
		return;

	i	= 0;
	while (positions->FindRect("frame",i,&positionFrame) == B_OK) {
		positionFrame.OffsetBy(delta);
		positions->ReplaceRect("frame",i,positionFrame);
		i++;
	}
}


int32 LayoutAppendSubCommands(BMessage *positions, BMessage *target)
{
	int32		i				= 0;
	void		*nodePtr		= NULL;
	BRect		newFrame;
	int32		subCommandCount	= 0;
	while (positions->FindPointer("node",i,&nodePtr) == B_OK) {
		if (positions->FindRect("frame",i,&newFrame) == B_OK) {
			BMessage	*subCommand		= new BMessage(P_C_EXECUTE_COMMAND);
			BMessage	*valueContainer	= new BMessage();
			subCommand->AddString("Command::Name","ChangeValue");
			subCommand->AddPointer("node",nodePtr);
			valueContainer->AddString("name",P_C_NODE_FRAME);
			valueContainer->AddInt32("type",(int32)B_RECT_TYPE);
			valueContainer->AddRect("newValue",newFrame);
			subCommand->AddMessage("valueContainer",valueContainer);
			target->AddMessage("PCommand::subPCommand",subCommand);
			subCommandCount++;
		}
		i++;
	}
	return subCommandCount;
}


BMessage* LayoutBuildBatchCommand(BMessage *positions)
{
	// One "Batch" wrapper, one ChangeValue subPCommand per node - single
	// undo step (see #116).
	BMessage	*wrapper	= new BMessage(P_C_EXECUTE_COMMAND);
	wrapper->AddString("Command::Name","Batch");

	if (LayoutAppendSubCommands(positions,wrapper) == 0) {
		delete wrapper;
		return NULL;
	}
	return wrapper;
}
