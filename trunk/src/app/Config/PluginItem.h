#ifndef PLUGINITEM_H
#define PLUGINITEM_H

class PluginItem : public BListItem
{
public:
                                PluginItem(BasePlugin *newNode, uint32 level = 0, bool expanded = true);
        virtual	BMessage*	GetNode(void){return node;};
        virtual void		SetNode(BMessage *newNode){node=newNode;};

        virtual void		ValueChanged(void);

        virtual	void		Select(void);
        virtual	void		Deselect(void);
        virtual void		SetExpanded(bool expande);
        virtual void		Update(BView *owner, const BFont *font);
        virtual	void		DrawItem(BView *owner, BRect bounds, bool complete = false);

        BList		*tmpPlugins = new BList();
        BasePlugin	*aktPlugin	= NULL;

};

#endif // PLUGINITEM_H
