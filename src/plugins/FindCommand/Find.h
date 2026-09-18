#ifndef FIND_H
#define FIND_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class Find : public PCommand
{

public:
							Find();
	
	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	// shows a Modal Window 
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Find";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
	/** `scope` is one of "nodes" (default), "connections", "both" - see
	 * kFindScopeNodes/Connections/Both in Find.cpp. The actual per-node
	 * string search itself is NodeSearch.h's NodeMatchesSearch(), shared
	 * with the If command (#135) via libProjectConceptor.so. */
	virtual	BList*			FindNodes(PDocument *doc,BString *searchTerm,const BString &scope);

	/*virtual	void			DoFind(PDocument *doc ,BRect *rect);
	virtual	void			DoFind(PDocument *doc ,BMessage *container);
	virtual void			DoFindAll(PDocument *doc);*/
private:
	//----------------PCommand
};
#endif
