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
	while (undo->FindPointer("node",i,(void **)&node) == B_OK)
	{
		err = B_OK;
		if (undo->FindMessage("valueContainer",i,valueContainer) == B_OK)
		{
			err = valueContainer->FindString("name",i,(const char**)&name);
			err = err | valueContainer->FindInt32("type",i,(int32 *)&type);
			err = err | valueContainer->FindInt32("index",i,&index);
			undoMessage->FindData("oldValue",type,(const void **)&oldValue,&oldSize);
			subGroupList->MakeEmpty();
			j	= 0;		
			subGroup = node;
			subGroupList->AddItem(subGroup);
			while (valueContainer->FindString("subgroup",j,(const char**)&subGroupName) == B_OK)
			{	
				subGroup->FindMessage(subGroupName,tmpSubGroup);
				subGroupList->AddItem(tmpSubGroup);
				subGroup	= tmpSubGroup;
				tmpSubGroup	= new BMessage();
				j++;
			}
			delete tmpSubGroup;
			subGroup->ReplaceData(name,type,index,oldValue,oldSize);
			for (j=subGroupList->CountItems()-1;j>0;j--)
			{
				tmpSubGroup = (BMessage *)subGroupList->ItemAt(j-1);
					valueContainer->FindString("subgroup",j-1,(const char**)&subGroupName);
				if (tmpSubGroup)
					tmpSubGroup->ReplaceMessage(subGroupName,(BMessage *)subGroupList->ItemAt(j));
				delete subGroupList->RemoveItem(j);
			}
			changed->insert(node);
		}
		i++;
	}
	i = 0;
	if ( (undo->FindBool("Node::selected",&selected) == B_OK) && (selected==true) )
	{
		undoMessage->FindMessage("selectedNodes",selectNodes);
		if (undo->FindMessage("valueContainer",i,valueContainer) == B_OK)
		{
			err = valueContainer->FindString("name",i,(const char**)&name);
			err = err | valueContainer->FindInt32("type",i,(int32 *)&type);
			err = err | valueContainer->FindInt32("index",i,(int32 *)&index);
			err 		= B_OK;
			int32 	i	= 0;
			while (selectNodes->FindPointer("node",i,(void **)&node) == B_OK)
			{
				j	= 0;
				subGroupList->MakeEmpty();
				subGroup = node;
				subGroupList->AddItem(subGroup);
				while (valueContainer->FindString("subgroup",j,(const char**)&subGroupName) == B_OK)
				{	
					subGroup->FindMessage(subGroupName,tmpSubGroup);
					subGroupList->AddItem(tmpSubGroup);
					subGroup	= tmpSubGroup;
					tmpSubGroup	= new BMessage();
					j++;
				}
				delete tmpSubGroup;
				selectNodes->FindData("oldValue",type,i,(const void **)&oldValue,&oldSize);
				subGroup->ReplaceData(name,type,index,oldValue,oldSize);
				for (j=subGroupList->CountItems()-1;j>0;j--)
				{
					tmpSubGroup = (BMessage *)subGroupList->ItemAt(j-1);
					valueContainer->FindString("subgroup",j-1,(const char**)&subGroupName);
					if (tmpSubGroup)
						tmpSubGroup->ReplaceMessage(subGroupName,(BMessage *)subGroupList->ItemAt(j));
					delete subGroupList->RemoveItem(j);
				}
				changed->insert(node);
				i++;
			}
		}
	}
	doc->SetModified();
}

BMessage* ChangeValue::Do(PDocument *doc, BMessage *settings)
{
	set<BMessage*>	*changed		= doc->GetChangedNodes();
	BList		*selection		= doc->GetSelected();
	BList		*subGroupList	= new BList();
	BMessage	*node			= NULL;
	BMessage	*undoMessage	= new BMessage();
	BMessage	*subGroup		= NULL;
	BMessage	*tmpSubGroup	= new BMessage();
	BMessage	*selectedNodes	= new BMessage();
	BMessage	*valueContainer	= new BMessage();
	int32		i				= 0;
	int32		j				= 0;
	bool		selected		= false;
	char		*name			= NULL;
	char		*subGroupName	= NULL;
	int32		index			= 0;
	type_code	type			= B_ANY_TYPE;
	void*		newValue		= NULL;
	void*		oldValue		= NULL;
	ssize_t		size			= 0;
	ssize_t		oldSize			= 0;
	status_t	err				= B_OK;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		if (settings->FindMessage("valueContainer",i,valueContainer) == B_OK)
		{
			err = valueContainer->FindString("name",(const char**)&name);
			err = err | valueContainer->FindInt32("type",(int32 *)&type);
			err = err | valueContainer->FindInt32("index",(int32 *)&index);
			err = valueContainer->FindData("newValue", type,(const void **)&newValue, &size);
			subGroupList->MakeEmpty();
			j	= 0;		
			subGroup = node;
			subGroupList->AddItem(subGroup);
			while (valueContainer->FindString("subgroup",j,(const char**)&subGroupName) == B_OK)
			{	
				subGroup->FindMessage(subGroupName,tmpSubGroup);
				subGroupList->AddItem(tmpSubGroup);
				subGroup	= tmpSubGroup;
				tmpSubGroup	= new BMessage();
				j++;
			}
			delete tmpSubGroup;
			subGroup->FindData(name,type,index,(const void **)&oldValue,&oldSize);
			undoMessage->AddData("oldValue",type,oldValue,oldSize,false);
			subGroup->ReplaceData(name,type,index,newValue,size);
			for (j=subGroupList->CountItems()-1;j>0;j--)
			{
				tmpSubGroup = (BMessage *)subGroupList->ItemAt(j-1);
				valueContainer->FindString("subgroup",j-1,(const char**)&subGroupName);
				if (tmpSubGroup)
					tmpSubGroup->ReplaceMessage(subGroupName,(BMessage *)subGroupList->ItemAt(j));
				delete subGroupList->RemoveItem(j);
			}
			changed->insert(node);
		}
		i++;
	}
	if ( (settings->FindBool("Node::selected",&selected) == B_OK) && (selected==true) )
	{
		if (settings->FindMessage("valueContainer",i,valueContainer) == B_OK)
		{
			err = valueContainer->FindString("name",i,(const char**)&name);
			err = err | valueContainer->FindInt32("type",i,(int32 *)&type);
			err = err | valueContainer->FindInt32("index",i,(int32 *)&index);
			err = err | valueContainer->FindData("newValue",type,i,(const void **)&newValue,&size);
//			if ( (settings->FindString("subgroup",i,(const char**)&subGroupName) != B_OK))
//			subGroupName=NULL;
			for (int32 i=0;i<selection->CountItems();i++)
			{
				node =(BMessage *) selection->ItemAt(i);
				selectedNodes->AddPointer("node",node);
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
				subGroup->FindData(name,type,index,(const void **)&oldValue,&oldSize);
				selectedNodes->AddData("oldValue",type,oldValue,oldSize,false);
				subGroup->ReplaceData(name,type,index,newValue,size);
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
	}
	undoMessage->AddMessage("selectedNodes",selectedNodes);
	doc->SetModified();
	settings->RemoveName("ChangeValue::Undo");
	settings->AddMessage("ChangeValue::Undo",undoMessage);
	settings =PCommand::Do(doc,settings);
	return settings;
}



void ChangeValue::AttachedToManager(void) {
}

void ChangeValue::DetachedFromManager(void) {
}

void ChangeValue::DoChangeValue(PDocument *doc,BRect *rect,BMessage *settings) {
	type_code		type;
	settings->FindInt32("type",(int32 *)&type);
}
