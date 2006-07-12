#include "PDocSaver.h"
#include "PDocument.h"

#include <support/Archivable.h>

PDocSaver::PDocSaver(PDocument *doc,BEntry *saveEntry)
{
	Init();
	toSave = new BFile (openEntry,B_READ_ONLY);
	Load();
}

PDocSaver::~PDocSaver(void)
{
}

void PDocSaver::Init(void)
{
	toSave	= NULL;
}


void PDocSaver::Save(void)
{
}

