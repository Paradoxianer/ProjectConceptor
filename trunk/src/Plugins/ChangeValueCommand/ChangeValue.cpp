#include "ProjectConceptorDefs.h"
#include "ChangeValue.h"


ChangeValue::ChangeValue():PCommand()
{
}

void ChangeValue::Undo(PDocument *doc,BMessage *undo)
{
	BMessage	*undoMessage	= new BMessage();
	BMessage	*subGroup		= new BMessage();
	BList		*changed		= doc->GetChangedNodes();
	BMessage	*node			= NULL;
	char		*name			= NULL;
	char		*subGroupName	= NULL;
	bool		selected		= false;
	int32		index			= 0;
	type_code	type			= B_ANY_TYPE;
	int32		i				= 0;
	status_t	err				= B_OK;
	void		*oldValue		= NULL;
	ssize_t		oldSize			= 0;
	PCommand::Undo(doc,undoMessage);
	undo->FindMessage("ChangeValue::Undo" ,undoMessage);
	while (undo->FindPointer("node",i,(void **)&node) == B_OK)
	{
		err = B_OK;
		err = undo->FindString("name",i,(const char**)&name);
		err = err | undo->FindInt32("type",i,(int32 *)&type);
		err = err | undo->FindInt32("index",i,&index);
		undoMessage->FindData("oldValue",type,(const void **)&oldValue,&oldSize);
		if ( (undo->FindString("subgroup",i,(const char**)&subGroupName) == B_OK) && (node->FindMessage(subGroupName,subGroup) == B_OK) )
		{
			subGroup->ReplaceData(name,type,index,oldValue,oldSize);
			node->ReplaceMessage(subGroupName,subGroup);
		}
		else
			node->ReplaceData(name,type,index,oldValue,oldSize);
		i++;
		changed->AddItem(node);
	}
	if ( (undo->FindBool("selected",&selected) == B_OK) && (selected==true) )
	{
		err 		= B_OK;
		int32 	i	= 0;
		err = undo->FindString("name",i,(const char**)&name);
		err = err | undo->FindInt32("type",i,(int32 *)&type);
		err = err | undo->FindInt32("index",i,&index);
		if ( (undo->FindString("subgroup",i,(const char**)&subGroupName) != B_OK))
			subGroupName=NULL;	
		while (undoMessage->FindPointer("seletion::node",i,(void **)&node) == B_OK)
		{
			changed->AddItem(node);
			undoMessage->FindData("selection::oldValue",type,i,(const void **)&oldValue,&oldSize);
			if (subGroupName)
			{
				node->FindMessage(subGroupName,subGroup);
				subGroup->ReplaceData(name,type,index,oldValue,oldSize);
				node->ReplaceMessage(subGroupName,subGroup);
			}
			else
				node->ReplaceData(name,type,index,oldValue,oldSize);
				i++;
		}
	}
	doc->SetModified();
}

BMessage* ChangeValue::Do(PDocument *doc, BMessage *settings)
{
	BList		*changed		= doc->GetChangedNodes();
	BList		*selection		= doc->GetSelected();
	BMessage	*undoMessage	= new BMessage();
	BMessage	*node			= NULL;
	BMessage	*subGroup		= new BMessage();
	int32		i				= 0;
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
		node->PrintToStream();
		err = B_OK;
		err = settings->FindString("name",i,(const char**)&name);
		err = err | settings->FindInt32("type",i,(int32 *)&type);
		err = err | settings->FindInt32("index",i,(int32 *)&index);
		err = err | settings->FindPointer("newValue",i,&newValue);
		err = err | settings->FindInt32("size",i,(int32 *)&size);
		if ( (settings->FindString("subgroup",i,(const char**)&subGroupName) == B_OK))
		{
			err = node->FindMessage(subGroupName,subGroup);
			err = err | subGroup->FindData(name,type,index,(const void **)&oldValue,&oldSize);
			undoMessage->AddData("oldValue",type,oldValue,oldSize,false);
			if (err==B_OK) 
				subGroup->ReplaceData(name,type,index,newValue,size);
			node->ReplaceMessage(subGroupName,subGroup);
		}
		else
		{
			err = err | node->FindData(name,type,index,(const void **)&oldValue,&oldSize);
			undoMessage->AddData("oldValue",type,oldValue,oldSize,false);
			if (err==B_OK) 
				node->ReplaceData(name,type,index,newValue,size);
		}
		node->PrintToStream();
		changed->AddItem(node);
		i++;
	}
	if ( (settings->FindBool("selected",&selected) == B_OK) && (selected==true) )
	{
		err = settings->FindString("name",i,(const char**)&name);
		err = err | settings->FindInt32("type",i,(int32 *)&type);
		err = err | settings->FindInt32("index",i,(int32 *)&index);
		err = err | settings->FindPointer("newValue",i,&newValue);
		err = err | settings->FindInt32("size",i,(int32 *)&size);
		if ( (settings->FindString("subgroup",i,(const char**)&subGroupName) != B_OK))
		subGroupName=NULL;
		for (int32 i=0;i<selection->CountItems();i++)
		{
			node =(BMessage *) selection->ItemAt(i);
			undoMessage->AddPointer("seletion::node",node);
			changed->AddItem(node);
			if (subGroupName)
			{
				err = node->FindMessage(subGroupName,subGroup);
				err = err | subGroup->FindData(name,type,index,(const void **)&oldValue,&oldSize);
				undoMessage->AddData("selection::oldValue",type,oldValue,oldSize,false);
				if (err==B_OK) 
					subGroup->ReplaceData(name,type,index,newValue,size);
				node->ReplaceMessage(subGroupName,subGroup);
			}
			else
			{
				err = err | node->FindData(name,type,index,(const void **)&oldValue,&oldSize);
				undoMessage->AddData("selection::oldValue",type,oldValue,oldSize,false);
				if (err==B_OK) 
					node->ReplaceData(name,type,index,newValue,size);
			}
		}
	}
	doc->SetModified();
	settings->RemoveName("ChangeValue::Undo");
	settings->AddMessage("ChangeValue::Undo",undoMessage);
	settings =PCommand::Do(doc,settings);
	return settings;
}



void ChangeValue::AttachedToManager(void)
{
}

void ChangeValue::DetachedFromManager(void)
{
}

void ChangeValue::DoChangeValue(PDocument *doc,BRect *rect,BMessage *settings)
{
	type_code		type;
	settings->FindInt32("type",(int32 *)&type);
}
