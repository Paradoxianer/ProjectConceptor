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
#include "MessageXmlReader.h"
#include "MessageXmlWriter.h"

status_t Identify(BPositionIO * inSource, const translation_format * inFormat,	BMessage * ioExtension,	translator_info * outInfo, uint32 outType)
{
	if ((!inSource) || (!outInfo))
		return B_BAD_VALUE;

	// Identify() runs against every file any app on the system scans via
	// BTranslatorRoster, not just ProjectConceptor documents - fully parsing
	// the whole stream before any cheap check meant every unrelated file got
	// fully parsed just to be rejected. Of the two formats we produce,
	// BMessage::Flatten()'s own binary format always starts with this 4-byte
	// magic, and the XML export always starts with an XML declaration -
	// reject anything matching neither before touching the rest of the
	// stream at all.
	char	prefix[16];
	off_t	savedPos	= inSource->Position();
	ssize_t	bytesRead	= inSource->Read(prefix, sizeof(prefix));
	inSource->Seek(savedPos, SEEK_SET);

	bool	looksNative	= (bytesRead >= 4) && (memcmp(prefix, "HMF1", 4) == 0);
	bool	looksXml	= (bytesRead >= 5) && (memcmp(prefix, "<?xml", 5) == 0);

	if (looksNative) {
		BMessage	testMessage;
		BMessage	tmpMessage;
		if ((testMessage.Unflatten(inSource) != B_OK)
			|| (testMessage.FindMessage("PDocument::allNodes",&tmpMessage) != B_OK))
			return B_NO_TRANSLATOR;
		outInfo->group = B_TRANSLATOR_NONE;
		outInfo->type = P_C_DOCUMENT_RAW_TYPE;
		outInfo->quality = 0.3;
		outInfo->capability = 1.0;
		strcpy(outInfo->name, "ProjectConceptor nativ format");
		strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
		return B_OK;
	}

	if (looksXml) {
		MessageXmlReader	xmlReader;
		BMessage			*parsed	= xmlReader.ReadFrom(inSource);
		inSource->Seek(savedPos, SEEK_SET);
		BMessage			tmpMessage;
		if ((parsed == NULL) || (parsed->FindMessage("PDocument::allNodes",&tmpMessage) != B_OK)) {
			delete parsed;
			return B_NO_TRANSLATOR;
		}
		delete parsed;
		outInfo->group = B_TRANSLATOR_TEXT;
		outInfo->type = P_C_DOCUMENT_TEXT_TYPE;
		outInfo->quality = 0.3;
		outInfo->capability = 1.0;
		strcpy(outInfo->name, "ProjectConceptor Text");
		strcpy(outInfo->MIME, "text/plain");
		return B_OK;
	}

	return B_NO_TRANSLATOR;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	BMessage		*allNodes			= new BMessage();
	BMessage		*allConnections		= new BMessage();
	BMessage		*selected			= new BMessage();
	BMessage		*commandStuff		= new BMessage();
	BMessage		*outCommand			= new BMessage();
	BMessage		*inMessage			= NULL;
	BMessage		*outMessage			= new BMessage();
	BMessage		*tmpMessage			= new BMessage();
	bool			saveUndo			= true;
	bool			saveMacro			= true;
	int32			undoLevel			= -1;
	printf("StandartTranslator::Translate\n");
	if (ioExtension != NULL)
	{
		ioExtension->FindBool("SaveUndo",&saveUndo);
		ioExtension->FindBool("SaveMacro",&saveUndo);
		ioExtension->FindInt32("UndoLevel",&undoLevel);
	}
	//necessary to avoid problems
	outDestination->Seek(0, SEEK_SET);
	inSource->Seek(0, SEEK_SET);

	// tInfo->type reflects whatever Identify() determined the *input*
	// stream actually is (native binary vs XML export) - read accordingly,
	// regardless of what output format was requested.
	if ((tInfo != NULL) && (tInfo->type == P_C_DOCUMENT_TEXT_TYPE)) {
		MessageXmlReader	xmlReader;
		inMessage = xmlReader.ReadFrom(inSource);
		if (inMessage == NULL)
			return B_NO_TRANSLATOR;
	} else {
		inMessage = new BMessage();
		err = inMessage->Unflatten(inSource);
		if (err != B_OK)
			return err;
	}
	//translations Process
	int32	formatVersion	= 0;
	if (inMessage->FindInt32(P_C_DOC_FORMAT_VERSION_FIELD,&formatVersion) == B_OK)
		outMessage->AddInt32(P_C_DOC_FORMAT_VERSION_FIELD,formatVersion);
	BMessage	*documentSetting	= new BMessage();
	inMessage->FindMessage("PDocument::documentSetting",documentSetting);
	outMessage->AddMessage("PDocument::documentSetting",documentSetting);
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
	DEBUG_ONLY(outMessage->PrintToStream(););

	if (outType == P_C_DOCUMENT_TEXT_TYPE) {
		MessageXmlWriter	xmlWriter;
		err = xmlWriter.WriteTo(*outMessage,outDestination);
	} else {
		err = outMessage->Flatten(outDestination);
	}
	//necessary to avoid problems
	inSource->Seek(0, SEEK_SET);
	outDestination->Seek(0, SEEK_SET);	/* paranoia */

	printf("StandartTranslator - Translate outMessage - %s\n", strerror(err));
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
