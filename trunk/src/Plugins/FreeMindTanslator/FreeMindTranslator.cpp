#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <ByteOrder.h>
#include <Message.h>
#include <Screen.h>
#include <Locker.h>
#include <FindDirectory.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <CheckBox.h>
#include <Bitmap.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "FreeMindTranslator.h"
#include "SettingsManager.h"
#include "ConfigView.h"

status_t Identify(BPositionIO * inSource, const translation_format * inFormat,	BMessage * ioExtension,	translator_info * outInfo, uint32 outType)
{
	status_t err	= B_OK;
/*	BMessage		*testMessage	= new BMessage();
	BMessage		*tmpMessage		= new BMessage();
	if ((!inSource) || (!outInfo))
		err = B_BAD_VALUE;
	else
	{
		if (err == B_OK)
		{
			err = testMessage->Unflatten(inSource);
			if (err == B_OK)
			{
				if (err == B_OK)
					err = testMessage->FindMessage("PDocument::allNodes",tmpMessage);
				if (err != B_OK)
				{
					err = B_NO_TRANSLATOR;
				}
				else
				{
					outInfo->group = B_TRANSLATOR_NONE;
					outInfo->type = P_C_DOCUMENT_RAW_TYPE;
					outInfo->quality = 0.3;		
					outInfo->capability = 0.8;	
					strcpy(outInfo->name, "ProjectConceptor nativ format");
					strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
				}
			}
		}
	}*/
	err = B_NO_TRANSLATOR;
	return err;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	Converter	*converter	= new Converter(inSource,ioExtension,outDestination);
	converter->ConvertPDoc2FreeMind();
	return err;
}

status_t MakeConfig(BMessage * ioExtension,	BView * * outView, BRect * outExtent)
{
	status_t err	= B_OK;
	*outView = new ConfigView();
	return err;
}

status_t GetConfigMessage(BMessage * ioExtension)
{
	status_t err = B_OK;

	return err;
}

Converter::Converter(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination)
{
	in		= inSource;
	config	= ioExtension;
	out		= outDestination;
	//necessary to avoid problems
	outDestination->Seek(0, SEEK_SET);
	inSource->Seek(0, SEEK_SET);
	saveUndo			= false;
	saveMacro			= false;
	restoreWindowPos	= false;
	undoLevel			= -1;
	doneNode			= NULL;
	doneConnection		= NULL;
	allConnections		= NULL;
	allNodes			= NULL;
	selected			= NULL;
	commandManager		= NULL;
	if (config != NULL)
	{
		config->FindBool("SaveUndo",&saveUndo);
		config->FindBool("SaveMacro",&saveUndo);
		config->FindInt32("UndoLevel",&undoLevel);
	}
}

status_t Converter::ConvertPDoc2FreeMind()
{
	status_t		err					= B_OK;
	BMessage		*inMessage			= new BMessage();
	BMessage		*tmpMessage			= new BMessage();
	int32			i					= 0;
	int32			undoLevel			= -1;
	BMessage		*node				= new BMessage();
	char			*line				= new char[1024];
//** Alert lost of Data and Nodes without connection will not be ported 
	//necessary inits
	allConnections	= new BMessage();
	commandManager	= new BMessage();
	selected		= new BMessage();
	allNodes		= new BMessage();
	doneNode		= new BList();
	doneConnection	= new BList();

	//extract all stuff
	err = inMessage->Unflatten(in);
	inMessage->FindMessage("PDocument::allConnections",allConnections);
	inMessage->FindMessage("PDocument::commandManager",commandManager);
	inMessage->FindMessage("PDocument::selected",selected);
	inMessage->FindMessage("PDocument::allNodes",allNodes);	
	sprintf(line,"%s","<map version=\"0.8.0\">\n<!-- This File was gernerated by ProjectConceptor! - To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->\n");
	out->Write(line,strlen(line));
	if (allConnections->FindMessage("node",tmpMessage)==B_OK)
	{
		//stores alle Nodes we already have visited
		WriteConnection(tmpMessage);
	}
	sprintf(line,"%s","</map>");
	out->Write(line,strlen(line));	
		
	//translations Process 
/*	inMessage->FindMessage("PDocument::allNodes",tmpMessage);
	while (tmpMessage->FindMessage("node",node) == B_OK)
	{
		inMessage->FindMessage("PDocument::allConnections",allConnections);
		outMessage->AddMessage("PDocument::allConnections",allConnections);
		inMessage->FindMessage("PDocument::selected",selected);
		outMessage->AddMessage("PDocument::selected",selected);
	}
	inMessage->FindMessage("PDocument::commandManager",commandStuff);
	if (saveUndo)
	{
		if (undoLevel>0)
		{
			type_code	typeFound;
			int32		countFound;
			int32		i			= 0;
			commandStuff->GetInfo("undo",&typeFound,&countFound);
			for (i =undoLevel;i< countFound;i++)
			{
				commandStuff->FindMessage("und",i,tmpMessage);
				outCommand->AddMessage("undo",tmpMessage);
			}
		}
	}
	else
		commandStuff->RemoveName("undo");
	if (!saveMacro)
		commandStuff->RemoveName("makro");
	outMessage->AddMessage("PDocument::commandManager",commandStuff);
	outMessage->PrintToStream();
	err = outMessage->Flatten(outDestination);
	//necessary to avoid problems
	inSource->Seek(0, SEEK_SET);
	outDestination->Seek(0, SEEK_SET);*/
	
	printf("StandartTranslator - Flatten outMessage - %s\n", strerror(err));
	return err;
}

status_t Converter::ConvertFreeMind2PDoc()
{
}

status_t Converter::WriteConnection(BMessage *connection)
{
	printf("Connection:\n");
	connection->PrintToStream();
	BMessage	*fromNode		= NULL;
	BMessage	*toNode			= NULL;
	BMessage	*data			= new BMessage;
	char		*text			= NULL;
	char		*line			= new char[512];
	void		*selfPointer	= NULL;
	bool		closeNode		= false;
	connection->FindPointer("this",&selfPointer);
	doneConnection->AddItem(selfPointer);
	connection->FindPointer("Node::to" ,(void **)&fromNode);
	if (!doneNode->HasItem(fromNode))
	{
		fromNode->FindMessage("Node::Data",data);
		data->FindString("Name",(const char **)&text);
		sprintf(line,"<node ID=\"%ld\" TEXT=\"%s\">\n",fromNode,text);
		doneNode->AddItem(fromNode);
		out->Write(line,strlen(line));
		FindConnections(fromNode);
		closeNode;
	}
	connection->FindPointer("Node::to" ,(void **)&toNode);
	if (doneNode->HasItem(toNode))
	{
		sprintf(line,"<arrowlink DESTINATION=\"%ld\"/>\n",toNode,text);
		out->Write(line,strlen(line));
	}
	else
	{
		toNode->FindMessage("Node::Data",data);
		data->FindString("Name",(const char **)&text);
		sprintf(line,"<node ID=\"%ld\" TEXT=\"%s\">\n",toNode,text);
		doneNode->AddItem(toNode);
		out->Write(line,strlen(line));	
		//**check for all Connections with the same Node in To (or From)
		FindConnections(toNode);
		sprintf(line,"</node>\n");
		out->Write(line,strlen(line));
	}
	if (closeNode)
	{
		sprintf(line,"</node>\n");
		out->Write(line,strlen(line));
	}
}

status_t Converter::FindConnections(BMessage *node)
{
	BMessage	*connection		= new BMessage();
	BMessage	*from			= NULL;
	BMessage	*to				= NULL;
	//we dont check the first Connections because this ist the "root-Connection" at the Moment
	int32		i				= 1;
	void		*selfPointer	= NULL;
	while (allConnections->FindMessage("node",i,connection)==B_OK)
	{
		connection->FindPointer("this",&selfPointer);
		if (!doneConnection->HasItem(selfPointer))
		{
			connection->FindPointer("Node::from",(void **)&from);
			connection->FindPointer("Node::to",(void **)&to);
			if (from == node)
				WriteConnection(connection);
			else if (to == node)
				WriteConnection(connection);
			
		}
		i++;
	}
}

