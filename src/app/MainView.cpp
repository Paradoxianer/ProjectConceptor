#include <interface/ScrollView.h>
#include "ProjectConceptorDefs.h"
#include "MainView.h"
//#include "PDocument.h"
#include "PEditorManager.h"


MainView::MainView(PDocument *newDoc,
		   BRect rect,
		   const char *name,
		   button_width width,
		   uint32 resizingMode,
		   uint32 flags)
	   : BTabView(rect,name,width,resizingMode,flags)
{
	TRACE();
#ifdef B_ZETA_VERSION_1_0_0
	BView::SetDoubleBuffering(1);
#endif
	doc = newDoc;
}
MainView::~MainView()
{
	TRACE();
}
void MainView::AttachedToWindow()
{
	TRACE();
}
void MainView::Select(int32 tab)
{
	TRACE();
	int32 selection=Selection();
	BTabView::Select(tab);
	(doc->GetEditorManager())->SetInactivePEditor(selection);
	(doc->GetEditorManager())->SetActivePEditor(tab);
}

void MainView::Select(const BTab *tab)
{
	TRACE();
	bool	found		= false;
	int32	i			= 0;
	BTab 	*compareTab	= BTabView::TabAt(i);
	while ((!found) && ( compareTab))
	{
		i++;
		if (compareTab==tab)
			found = true;
		compareTab = BTabView::TabAt(i);
	}
	if (found) 
		BTabView::Select(i-1);
        PRINT(("Select(%ld)\n",i));
}
