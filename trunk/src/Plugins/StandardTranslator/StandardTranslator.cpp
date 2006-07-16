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
#include "StandardTranslator.h"
#include "SettingsManager.h"
#include "ConfigView.h"

status_t Identify(BPositionIO * inSource, const translation_format * inFormat,	BMessage * ioExtension,	translator_info * outInfo, uint32 outType)
{
	status_t err	= B_OK;
	BMessage		*testMessage	= new BMessage();
	BMessage		*tmpMessage		= new BMessage();
	if ((!inSource) || (!outInfo))
		err = B_BAD_VALUE;
	else
	{
		if (outType == 0) 
			outType = B_TRANSLATOR_NONE;
		if (outType != B_TRANSLATOR_NONE && outType != P_C_DOCUMENT_TYPE)
			err	= B_NO_TRANSLATOR;
		err = testMessage->Unflatten(inSource);
		if (err == B_OK)
			err = testMessage->FindMessage("Document::allNodes",tmpMessage);
		if (err != B_OK)
			err = B_NO_TRANSLATOR;
		else
		{
			outInfo->group = B_NO_TRANSLATOR;
			outInfo->type = P_C_DOCUMENT_TYPE;
			outInfo->quality = 0.3;		
			outInfo->capability = 0.8;	
			strcpy(outInfo->name, "ProjectConceptor nativ format");
			strcpy(outInfo->MIME, "application/application/x-vnd.projectconceptor-document");
		}
	}
	return err;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	BMessage		*allNodes			= new BMessage();
	BMessage		*allConnections		= new BMessage();
	BMessage		*selected			= new BMessage();
	BMessage		*commandStuff		= new BMessage();
	BMessage		*outCommand			= new BMessage();
	BMessage		*inMessage			= new BMessage();
	BMessage		*outMessage			= new BMessage();
	BMessage		*tmpMessage			= new BMessage();
	bool			saveUndo			= true;
	bool			saveMacro			= true;
	bool			restoreWindowPos	= true;
	int32			undoLevel			= -1;

	if (ioExtension != NULL)
	{
		ioExtension->FindBool("SaveUndo",&saveUndo);
		ioExtension->FindBool("SaveMacro",&saveUndo);
		ioExtension->FindInt32("UndoLevel",&undoLevel);
	}

	inMessage->Unflatten(inSource);
	//translations Process 
	inMessage->FindMessage("PDocument::allNodes",allNodes);
	outMessage->AddMessage("PDocument::allNodes",allNodes);
	inMessage->FindMessage("PDocument::allConnections",allConnections);
	outMessage->AddMessage("PDocument::allConnections",allConnections);
	inMessage->FindMessage("PDocument::selected",selected);
	outMessage->AddMessage("PDocument::selected",selected);

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
	outMessage->Flatten(outDestination);
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
