#include "ProjectConceptorDefs.h"
#include "AddAttribute.h"


AddAttribute::AddAttribute():PCommand()
{
}

void AddAttribute::Undo(PDocument *doc,BMessage *undo)
{
	BMessage	*undoMessage	= new BMessage();
	BMessage	*subGroup		= new BMessage();
	BMessage	*selectNodes	= new BMessage();
	BList		*changed		= doc->GetChangedNodes();
	BMessage	*node			= NULL;
	char		*name			= NULL;
	char		*compareName	= NULL;
	char		*subGroupName	= NULL;
	bool		selected		= false;
	int32		index			= 0;
	int32		count			= 0;
	type_code	type			= B_ANY_TYPE;
	type_code	typeFound		= NULL;
	int32		lastIndex		= -1;
	int32		i				= 0;
	status_t	err				= B_OK;
	void		*dummyData		= NULL;
	ssize_t		oldSize			= 0;
	PCommand::Undo(doc,undoMessage);
	undo->FindMessage("AddAttribute::Undo" ,undoMessage);
	while (undo->FindPointer("node",i,(void **)&node) == B_OK)
	{
		err = B_OK;
		err = undo->FindString("name",i,(const char**)&name);
		err = err | undo->FindInt32("type",i,(int32 *)&type);
		lastIndex = -1;
		if ( (undo->FindString("subgroup",i,(const char**)&subGroupName) == B_OK) && (node->FindMessage(subGroupName,subGroup) == B_OK) )
		{
			index = 0;
			err = node->FindMessage(subGroupName,subGroup);
        	while (subGroup->GetInfo(B_ANY_TYPE, index, (const char **)&compareName, &typeFound, &count) == B_OK)
    		{
    			if ( (strcmp(name,compareName)) && (type == typeFound) )
    				lastIndex = count;
       		}  
			subGroup->RemoveData(name,lastIndex);
			node->ReplaceMessage(subGroupName,subGroup);
		}
		else
		{
			index = 0;
			while (node->GetInfo(B_ANY_TYPE, index,(const char**) &compareName, &typeFound, &count) == B_OK)
    		{
    			if ( (strcmp(name,compareName)) && (type == typeFound) )
    				lastIndex = count;
       		}  
			node->RemoveData(name,lastIndex);
		}
		i++;
		changed->AddItem(node);
	}
	if ( (undo->FindBool("selected",&selected) == B_OK) && (selected==true) )
	{
		undoMessage->FindMessage("selectedNodes",selectNodes);
		err 		= B_OK;
		int32 	i	= 0;
		err = undo->FindString("name",i,(const char**)&name);
		err = err | undo->FindInt32("type",i,(int32 *)&type);
		if ( (undo->FindString("subgroup",i,(const char**)&subGroupName) != B_OK))
			subGroupName=NULL;	
		while (selectNodes->FindPointer("node",i,(void **)&node) == B_OK)
		{
			lastIndex = -1;
			changed->AddItem(node);
			if (subGroupName)
			{
				index = 0;
				err = node->FindMessage(subGroupName,subGroup);
				while (subGroup->GetInfo(B_ANY_TYPE, index,(const char**) &compareName, &typeFound, &count) == B_OK)
    			{
    				if ( (strcmp(name,compareName)==B_OK) && (type == typeFound) )
    					lastIndex = count-1;
    				index++;
       			}  
				subGroup->RemoveData(name,lastIndex);
				node->ReplaceMessage(subGroupName,subGroup);
			}
			else
			{
				index = 0;
				while (node->GetInfo(B_ANY_TYPE, index,(const char**) &compareName, &typeFound, &count) == B_OK)
    			{
    				if ( (strcmp(name,compareName)==B_OK) && (type == typeFound) )
	    				lastIndex = count-1;
	    			index++;
    	   		}
    			node->RemoveData(name,lastIndex);
	   	   	}
			i++;
		}
	}
	doc->SetModified();
}

BMessage* AddAttribute::Do(PDocument *doc, BMessage *settings)
{
	BList		*changed		= doc->GetChangedNodes();
	BList		*selection		= doc->GetSelected();
	BMessage	*undoMessage	= new BMessage();
	BMessage	*node			= NULL;
	BMessage	*subGroup		= new BMessage();
	BMessage	*selectedNodes	= new BMessage();
	int32		i				= 0;
	bool		selected		= false;
	char		*name			= NULL;
	char		*subGroupName	= NULL;
	type_code	type			= B_ANY_TYPE;
	void*		newValue		= NULL;
	void*		oldValue		= NULL;
	ssize_t		size			= 0;
	ssize_t		oldSize			= 0;
	status_t	err				= B_OK;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		err = settings->FindString("name",i,(const char**)&name);
		err = err | settings->FindInt32("type",i,(int32 *)&type);
		err = settings->FindData("newAttribute", type, i, (const void **)&newValue, &size);
		if ( (settings->FindString("subgroup",i,(const char**)&subGroupName) == B_OK))
		{
			err = node->FindMessage(subGroupName,subGroup);
			if (err == B_OK) 
				subGroup->AddData(name,type,newValue,size);
			node->ReplaceMessage(subGroupName,subGroup);
		}
		else
		{
			if (err == B_OK) 
				node->AddData(name,type,newValue,size);
		}
		changed->AddItem(node);
		i++;
	}
	if ( (settings->FindBool("selected",&selected) == B_OK) && (selected==true) )
	{
		err = settings->FindString("name",i,(const char**)&name);
		err = err | settings->FindInt32("type",i,(int32 *)&type);
		err = err | settings->FindData("newAttribute",type,i,(const void **)&newValue,&size);
		if ( (settings->FindString("subgroup",i,(const char**)&subGroupName) != B_OK))
		subGroupName=NULL;
		for (int32 i=0;i<selection->CountItems();i++)
		{
			node =(BMessage *) selection->ItemAt(i);
			selectedNodes->AddPointer("node",node);
			changed->AddItem(node);
			if (subGroupName)
			{
				err = node->FindMessage(subGroupName,subGroup);
				if (err==B_OK) 
					subGroup->AddData(name,type,newValue,size);
				node->ReplaceMessage(subGroupName,subGroup);
			}
			else
			{
				if (err==B_OK) 
					node->AddData(name,type,newValue,size);
			}
		}
	}
	undoMessage->AddMessage("selectedNodes",selectedNodes);
	doc->SetModified();
	settings->RemoveName("AddAttribute::Undo");
	settings->AddMessage("AddAttribute::Undo",undoMessage);
	settings =PCommand::Do(doc,settings);
	return settings;
}



void AddAttribute::AttachedToManager(void)
{
}

void AddAttribute::DetachedFromManager(void)
{
}
