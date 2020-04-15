#include "ProjectConceptorDefs.h"
#include "ChangeValue.h"


ChangeValue::ChangeValue():PCommand()
{
}

void ChangeValue::Undo(PDocument *doc,BMessage *undo)
{
	BMessage	*undoMessage	= new BMessage();
	BMessage	*subGroup		= NULL;
	BMessage	*tmpSubGroup	= new BMessage();
	BMessage	*valueContainer	= new BMessage();
	BMessage	*selectNodes	= new BMessage();
	BList		*subGroupList	= new BList();
	set<BMessage*>	*changed		= doc->GetChangedNodes();
	BMessage	*node			= NULL;
	char		*name			= NULL;
	char		*subGroupName	= NULL;
	bool		selected		= false;
	int32		index			= 0;
	type_code	type			= B_ANY_TYPE;
	int32		j				= 0;
	int32		i				= 0;
	status_t	err				= B_OK;
	void		*oldValue		= NULL;
	ssize_t		oldSize			= 0;
	PCommand::Undo(doc,undoMessage);
	undo->FindMessage("ChangeValue::Undo" ,undoMessage);
	while (undoMessage->FindPointer("node",i,(void **)&node) == B_OK){
		err = B_OK;
		if (err = undo->FindMessage("valueContainer",i,valueContainer) != B_OK) {
			err = undo->FindMessage("valueContainer",0,valueContainer);
		}
		if (err == B_OK) {
			err = valueContainer->FindString("name",(const char**)&name);
			err = err || valueContainer->FindInt32("type",(int32 *)&type);
			err = err || valueContainer->FindInt32("index",(int32 *)&index);
			if (undoMessage->FindData("oldValue",type,i,(const void **)&oldValue,&oldSize) == B_OK) {
				subGroupList->MakeEmpty();
				j	= 0;		
				subGroup = node;
				subGroupList->AddItem(subGroup);
				while (valueContainer->FindString("subgroup",j,(const char**)&subGroupName) == B_OK) {	
					subGroup->FindMessage(subGroupName,tmpSubGroup);
					subGroupList->AddItem(tmpSubGroup);
					subGroup	= tmpSubGroup;
					tmpSubGroup	= new BMessage();
					j++;
				}
				delete tmpSubGroup;
				err = subGroup->ReplaceData(name,type,index,oldValue,oldSize);
				if (err != B_OK)
					fprintf(stderr,"Error %s -replacing %s valueContainer \n" ,strerror(err),name);
				for (j=subGroupList->CountItems()-1;j>0;j--) {
					tmpSubGroup = (BMessage *)subGroupList->ItemAt(j-1);
					valueContainer->FindString("subgroup",j-1,(const char**)&subGroupName);
					if (tmpSubGroup)
						tmpSubGroup->ReplaceMessage(subGroupName,(BMessage *)subGroupList->ItemAt(j));
					delete subGroupList->RemoveItem(j);
				}
				changed->insert(node);
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
	doc->SetModified();
	settings->RemoveName("ChangeValue::Undo");
	settings->AddMessage("ChangeValue::Undo",undoMessage);
	settings = PCommand::Do(doc,settings);
	return settings;
}

void ChangeValue::AttachedToManager(void)
{
}


void ChangeValue::DetachedFromManager(void)
{
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
	delete tmpSubGroup;
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
		delete pathList->RemoveItem(j);
	}
}
