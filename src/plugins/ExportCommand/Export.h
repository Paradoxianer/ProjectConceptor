#ifndef EXPORT_H
#define EXPORT_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <storage/Entry.h>
#include <support/String.h>

#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Export
 * @brief Writes a copy of the document, or of its selection, to "path"
 * in the format of the file name extension "format" (else the path's
 * own), like File > Export. A relative path lies next to the document's
 * file, "~/" in the home folder. An existing file is overwritten. The
 * document doesn't change, so there is nothing to undo.
 */
class Export : public PCommand
{

public:
							Export();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Export";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

	/** the file and the format settings describes; error tells why not */
	static	status_t		Target(PDocument *doc, const BMessage &settings,
								entry_ref *ref, BMessage *format,
								BString *error);
protected:
private:
	//----------------PCommand
};
#endif
