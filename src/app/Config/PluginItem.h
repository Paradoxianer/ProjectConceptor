#ifndef PLUGINITEM_H
#define PLUGINITEM_H

#include <Bitmap.h>
#include <ListItem.h>
#include "BasePlugin.h"
#include "PluginManager.h"


class PluginItem : public BListItem
{
public:
				PluginItem(BasePlugin *newPlugin, PluginManager *pluginManager, uint32 level = 0, bool expanded = true);
	virtual	BasePlugin*	GetPlugin(void){return plugin;}
	virtual void		SetPlugin(BasePlugin *newPlugin){plugin=newPlugin;}

/*        virtual void		ValueChanged(void);

        virtual	void		Select(void);
        virtual	void		Deselect(void);
	virtual void		SetExpanded(bool expande);*/
        virtual void		Update(BView *owner, const BFont *font);
        virtual	void		DrawItem(BView *owner, BRect bounds, bool complete = false);

    private:
	BasePlugin	*plugin;
	BBitmap		*icon;
	float		baseLine;


};

#endif // PLUGINITEM_H
