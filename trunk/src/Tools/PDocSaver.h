#ifndef P_DOC_SAVER_H
#define P_DOC_SAVER_H

#include <app/Message.h>
#include <storage/Entry.h>
#include <storage/File.h>

#include "Commands/PCommandManager.h"
#include "Document/PEditorManager.h"
#include "Document/WindowManager.h"

class PDocument;
/**
 * @class PDocSaver
 *
 * @brief  PDocLoader is a class to save ProjectConceptor files
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@projectconceptor.de
 *
 *
 */

class PDocSaver
{

public:
								PDocSaver(PDocument *doc,BEntry *saveEntry);
	virtual						~PDocSaver(void);
	virtual	void				Save(void);	
	
protected:
	virtual void				Init(void);
			
		BFile*					toSave;

private:

};
#endif
