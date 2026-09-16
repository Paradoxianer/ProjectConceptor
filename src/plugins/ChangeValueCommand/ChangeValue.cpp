#include <Debug.h>

#include "ProjectConceptorDefs.h"
#include "ChangeValue.h"


ChangeValue::ChangeValue():PCommand()
{
}

static const property_info kChangeValueProperties[] = {
	// included_node: see Insert.cpp's kInsertProperties for why any
	// command with a "node" field can carry this.
	{ "ChangeValue", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Sets one field on a node - valueContainer holds the field name, "
		"type and new value (see the paired \"node\").", 0, {0},
		{ { { {"node", B_POINTER_TYPE}, {"valueContainer", B_MESSAGE_TYPE},
			  {"included_node", B_MESSAGE_TYPE} } } } },
};

const property_info* ChangeValue::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kChangeValueProperties;
}

void ChangeValue::Undo(PDocument *doc,BMessage *undo)
{
	BMessage	*undoMessage	= new BMessage();
	BMessage	*valueContainer	= new BMessage();
	BList		*subGroupList	= new BList();
	set<BMessage*>	*changed		= doc->GetChangedNodes();
	BMessage	*node			= NULL;
	int32		i				= 0;
	status_t	err				= B_OK;
	type_code	type			= B_ANY_TYPE;
	void		*oldValue		= NULL;
	ssize_t		oldSize			= 0;
	PCommand::Undo(doc,undo);
	undo->FindMessage("ChangeValue::Undo" ,undoMessage);
	while (undoMessage->FindPointer("node",i,(void **)&node) == B_OK){
		err = B_OK;
		if (err = undo->FindMessage("valueContainer",i,valueContainer) != B_OK) {
			err = undo->FindMessage("valueContainer",0,valueContainer);
		}
		if (err == B_OK) {
			if (undoMessage->FindData("oldValue",type,i,(const void **)&oldValue,&oldSize) == B_OK) {
				UndoChangeValue(node, valueContainer,type,oldValue, oldSize);
				changed->insert(node);
			} else {
				fprintf(stderr, "ChangeValue::Undo couldnt find oldValue\n");
			}
			
		}
		else {
			fprintf(stderr, "ChangeValue::Undo couldnt find the valueContainer\n");
		}
		i++;
	}
	doc->SetModified();
}

BMessage* ChangeValue::Do(PDocument *doc, BMessage *settings)
{
	set<BMessage*>	*changed		= doc->GetChangedNodes();	
	BList		*selection		= doc->GetSelected();
	BMessage	*node			= NULL;
	BMessage	*undoMessage	= new BMessage();
	BMessage	*selectedNodes	= new BMessage();
	BMessage	*valueContainer	= new BMessage();
	int32		i				= 0;
	bool		selected		= false;
	
	status_t	err				= B_OK;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		if (settings->FindMessage("valueContainer",i,valueContainer) == B_OK) {
			DoChangeValue(node,valueContainer,undoMessage);
			changed->insert(node);
		}
		i++;
	}
	if ( (settings->FindBool(P_C_NODE_SELECTED,&selected) == B_OK) && (selected==true) ) {
		if (settings->FindMessage("valueContainer",i,valueContainer) == B_OK) {
			for (i=0;i<selection->CountItems();i++) {
				node =(BMessage *) selection->ItemAt(i);
				DoChangeValue(node,valueContainer, undoMessage);
				changed->insert(node);
			}
		}
	}
	settings->RemoveName("ChangeValue::Undo");
	settings->AddMessage("ChangeValue::Undo",undoMessage);
	settings = PCommand::Do(doc,settings);
	doc->SetModified();
	printf("ChangeValue::Do()\n");
	settings->PrintToStream();
	return settings;
}

void ChangeValue::AttachedToManager(void)
{
}


void ChangeValue::DetachedFromManager(void)
{
}


void ChangeValue::UndoChangeValue(BMessage *node,BMessage *valueContainer, type_code type,void *oldValue, ssize_t oldSize)
{
	status_t	err				= B_OK;
	BList		*pathList		= new BList();
	int32 		j				= 0;
	const char	*name			= NULL;
	int32		index			= 0;
	BMessage	*subGroup		= NULL;
	BMessage	*tmpSubGroup	= new BMessage();
	char		*subGroupName	= NULL;

	err = valueContainer->FindString("name",&name);
	err = err || valueContainer->FindInt32("type",(int32 *)&type);
	err = err || valueContainer->FindInt32("index",(int32 *)&index);
	subGroup = node;
	pathList->AddItem(subGroup);
	while (valueContainer->FindString("subgroup",j,(const char**)&subGroupName) == B_OK) {
		subGroup->FindMessage(subGroupName,tmpSubGroup);
		pathList->AddItem(tmpSubGroup);
		subGroup	= tmpSubGroup;
		tmpSubGroup	= new BMessage();
		j++;
	}
	err = subGroup->ReplaceData(name,type,index,oldValue,oldSize);
	if (err != B_OK)
		fprintf(stderr,"Error %s -replacing %s valueContainer \n" ,strerror(err),name);
	for (j=pathList->CountItems()-1;j>0;j--) {
		tmpSubGroup = (BMessage *)pathList->ItemAt(j-1);
		valueContainer->FindString("subgroup",j-1,(const char**)&subGroupName);
		if (tmpSubGroup != NULL)
			tmpSubGroup->ReplaceMessage(subGroupName,(BMessage *)pathList->ItemAt(j));
	}
}

void ChangeValue::DoChangeValue(BMessage *node,BMessage *valueContainer, BMessage *undo)
{
	BList			*pathList		= new BList();
	BMessage		*subGroup		= NULL;
	BMessage		*tmpSubGroup	= new BMessage();
	status_t		err				= B_OK;
	const char		*name			= NULL;
	const char		*subGroupName	= NULL;
	int32			index			= 0;
	int32			j				= 0;

	type_code		type			= B_ANY_TYPE;
	const void		*newValue		= NULL;
	const void		*oldValue		= NULL;
	ssize_t			size			= 0;
	ssize_t			oldSize			= 0;
	
	err = valueContainer->FindString("name",&name);
	err = err | valueContainer->FindInt32("type",(int32 *)&type);
	err = err | valueContainer->FindInt32("index",(int32 *)&index);
	err = valueContainer->FindData("newValue", type,&newValue, &size);
	
	//store the pointer of the changed node
	undo->AddPointer("node",node);
	pathList->MakeEmpty();
	j	= 0;
	subGroup = node;
	pathList->AddItem(subGroup);
	while (valueContainer->FindString("subgroup",j,&subGroupName) == B_OK){	
		subGroup->FindMessage(subGroupName,tmpSubGroup);
		pathList->AddItem(tmpSubGroup);
		subGroup	= tmpSubGroup;
		tmpSubGroup	= new BMessage();
		j++;
	}
	err = subGroup->FindData(name,type,index,(const void **)&oldValue,&oldSize);
	if (err != B_OK)
		fprintf(stderr,"Error finding oldValue %s\n",strerror(err));
	undo->AddData("oldValue",type,oldValue,oldSize,false);
	err = subGroup->ReplaceData(name,type,index,newValue,size);
	for (j=pathList->CountItems()-1;j>0;j--) {
		tmpSubGroup = (BMessage *)pathList->ItemAt(j-1);
		valueContainer->FindString("subgroup",j-1,(const char**)&subGroupName);
		if (tmpSubGroup)
			err=tmpSubGroup->ReplaceMessage(subGroupName,(BMessage *)pathList->ItemAt(j));
	}
}
