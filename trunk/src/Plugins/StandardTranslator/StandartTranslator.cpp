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
#include "StandartTranslator.h"
#include "SettingsManager.h"


status_t Identify(BPositionIO * inSource, const translation_format * inFormat,	BMessage * ioExtension,	translator_info * outInfo, uint32 outType)
{
	status_t err	= B_OK;
	return err;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	BMessage		*allNodes			= new BMessage();
	BMessage		*allConnections		= new BMessage();
	BMessage		*selected			= new BMessage();
	BMessage		*commandStuff		= new BMessage();
	SettingsManager	*settingsManager	= new SettingsManager();
	BMessage		*inMessage			= new BMessage();
	//translations Process 
	inMessage->FindMessage("PDocument::allNodes",allNodes);
	outMessage->AddMessage("PDocument::allNodes",allNodes);
	inMessage->FindMessage("PDocument::allConnections",allConnections);
	outMessage->AddMessage("PDocument::allConnections",allConnections);
	inMessage->FindMessage("PDocument::selected",selected);
	outMessage->AddMessage("PDocument::selected",selected);
	
	inMessage->FindMessage("PDocument::commandManager",commandStuff);
	outMessage->AddMessage("PDocument::commandManager",commandStuff);
	BMessage		*outMessage			= new BMessage();
	inMessage->Unflatten(inSource);
	
	outMessage->Flatten(outDestination);
	return err;
}

status_t MakeConfig(BMessage * ioExtension,	BView * * outView, BRect * outExtent)
{
	status_t err	= B_OK;
	return err;
}

status_t GetConfigMessage(BMessage * ioExtension)
{
	status_t err = B_OK;
	return err;
}
