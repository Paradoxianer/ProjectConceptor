#include <List.h>
#include "ToolBar.h"

class InfoToolBarManager{

public:
					InfoToolBarManager(BList *_selected, BHandler *_target);
	void			ValueChanged(void);
	ToolBar*		InfoToolBar(void){return infoToolBar;};

private:
	void			Init(void);
	BList*			selected;
	ToolBar*		infoToolBar;
	FloatToolItem	*left;
	FloatToolItem	*top;
	FloatToolItem	*width;
	FloatToolItem	*height;
};
