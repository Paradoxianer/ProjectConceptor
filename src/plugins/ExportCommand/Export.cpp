#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <Catalog.h>

#include "Export.h"
#include "PCommandManager.h"
#include "PCSavePanel.h"
#include "ProjectConceptorDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Export"


Export::Export():PCommand()
{
}

static const property_info kExportProperties[] = {
	{ "Export", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Writes a copy of the document to \"path\" (required) in the "
		"format of the file name extension \"format\" (optional, else the "
		"path's own, e.g. html, svg, png, drawio). A relative path lies "
		"next to the document, ~/ in the home folder. selection=true "
		"writes only the selected nodes.", 0, {0},
		{ { { {P_C_EXPORT_PATH, B_STRING_TYPE}, {P_C_EXPORT_FORMAT, B_STRING_TYPE},
			  {P_C_EXPORT_SELECTION, B_BOOL_TYPE} } } } },
};

const property_info* Export::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kExportProperties;
}


status_t Export::Target(PDocument *doc, const BMessage &settings,
	entry_ref *ref, BMessage *format, BString *error)
{
	BString	path(settings.GetString(P_C_EXPORT_PATH,""));
	if (path.Length() == 0) {
		*error	= B_TRANSLATE("Export needs a file in \"path\".");
		return B_BAD_VALUE;
	}
	BString	full;
	if ((path == "~") || path.StartsWith("~/")) {
		BPath	home;
		status_t	err	= find_directory(B_USER_DIRECTORY,&home);
		if (err != B_OK) {
			error->SetToFormat(B_TRANSLATE("Could not find the home folder: %s"),
				strerror(err));
			return err;
		}
		full << home.Path() << (path.String() + 1);
	} else if (path.StartsWith("/")) {
		full	= path;
	} else {
		BEntry	directory;
		BPath	directoryPath;
		if ((doc->Entry() == NULL)
			|| (BEntry(doc->Entry()).GetParent(&directory) != B_OK)
			|| (directory.GetPath(&directoryPath) != B_OK)) {
			error->SetToFormat(B_TRANSLATE("\"%s\" lies next to the document, "
				"which isn't saved yet."),path.String());
			return B_BAD_VALUE;
		}
		full << directoryPath.Path() << "/" << path;
	}

	BString	extension(settings.GetString(P_C_EXPORT_FORMAT,""));
	if (extension.Length() == 0) {
		int32	dot		= full.FindLast('.');
		int32	slash	= full.FindLast('/');
		if (dot > slash + 1)
			extension.SetTo(full.String() + dot + 1);
	}
	if (extension.Length() == 0) {
		error->SetToFormat(B_TRANSLATE("\"%s\" has no extension to tell the "
			"format by, and \"format\" is empty."),full.String());
		return B_BAD_VALUE;
	}
	if (PCSavePanel::FindFormat(extension.String(),format) != B_OK) {
		error->SetToFormat(B_TRANSLATE("There is no format for \"%s\"."),
			extension.String());
		return B_NAME_NOT_FOUND;
	}

	status_t	err	= BEntry(full.String()).GetRef(ref);
	if (err != B_OK) {
		error->SetToFormat(B_TRANSLATE("Could not write to \"%s\": %s"),
			full.String(),strerror(err));
		return err;
	}
	return B_OK;
}


BMessage* Export::Do(PDocument *doc, BMessage *settings)
{
	entry_ref	ref;
	BMessage	format;
	BString		error;
	if ((Target(doc,*settings,&ref,&format,&error) != B_OK)
		|| (doc->WriteTo(ref,format,true,settings->GetBool(P_C_EXPORT_SELECTION,false),
			&error) != B_OK)) {
		BString	problem;
		problem << "Export: " << error;
		manager->AddPlaybackError(problem);
	}
	return PCommand::Do(doc,settings);
}

void Export::Undo(PDocument *doc,BMessage *undo)
{
	// the document didn't change
}


void Export::AttachedToManager(void)
{
}

void Export::DetachedFromManager(void)
{
}
