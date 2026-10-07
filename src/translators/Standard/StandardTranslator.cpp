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
#include "ConfigView.h"
#include "MessageXmlReader.h"
#include "MessageXmlWriter.h"

// 0 asks for the translator's default format (native)
static bool CanWrite(uint32 outType)
{
	return (outType == 0) || (outType == P_C_DOCUMENT_TYPE)
		|| (outType == P_C_DOCUMENT_RAW_TYPE) || (outType == P_C_DOCUMENT_TEXT_TYPE);
}

status_t Identify(BPositionIO * inSource, const translation_format * inFormat,	BMessage * ioExtension,	translator_info * outInfo, uint32 outType)
{
	if ((!inSource) || (!outInfo))
		return B_BAD_VALUE;
	if (!CanWrite(outType))
		return B_NO_TRANSLATOR;

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
	// refuse rather than falling through to the binary format below
	if (!CanWrite(outType))
		return B_NO_TRANSLATOR;

	outDestination->Seek(0, SEEK_SET);
	inSource->Seek(0, SEEK_SET);

	// tInfo->type is what Identify() found the *input* to be (native binary
	// or XML export), independent of the requested output format
	BMessage	inMessage;
	if ((tInfo != NULL) && (tInfo->type == P_C_DOCUMENT_TEXT_TYPE)) {
		MessageXmlReader	xmlReader;
		BMessage			*parsed	= xmlReader.ReadFrom(inSource);
		if (parsed == NULL)
			return B_NO_TRANSLATOR;
		inMessage = *parsed;
		delete parsed;
	} else {
		status_t	err	= inMessage.Unflatten(inSource);
		if (err != B_OK)
			return err;
	}

	// only the document's own parts go out; a missing part is written empty
	BMessage	outMessage;
	int32		formatVersion	= 0;
	if (inMessage.FindInt32(P_C_DOC_FORMAT_VERSION_FIELD,&formatVersion) == B_OK)
		outMessage.AddInt32(P_C_DOC_FORMAT_VERSION_FIELD,formatVersion);
	const char	*parts[]	= { "PDocument::documentSetting", "PDocument::allNodes",
		"PDocument::allConnections", "PDocument::selected",
		"PDocument::commandManager" };
	for (size_t i = 0; i < sizeof(parts) / sizeof(parts[0]); i++) {
		BMessage	part;
		inMessage.FindMessage(parts[i],&part);
		outMessage.AddMessage(parts[i],&part);
	}

	status_t	err;
	if (outType == P_C_DOCUMENT_TEXT_TYPE) {
		MessageXmlWriter	xmlWriter;
		err = xmlWriter.WriteTo(outMessage,outDestination);
	} else {
		err = outMessage.Flatten(outDestination);
	}
	inSource->Seek(0, SEEK_SET);
	outDestination->Seek(0, SEEK_SET);
	return err;
}

status_t MakeConfig(BMessage * ioExtension,	BView * * outView, BRect * outExtent)
{
	status_t err	= B_OK;
	*outView = new ConfigView(translatorName);
	return err;
}

status_t GetConfigMessage(BMessage * ioExtension)
{
	status_t err = B_OK;

	return err;
}
