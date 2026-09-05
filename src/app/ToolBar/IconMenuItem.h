#ifndef ICON_MENU_ITEM_H
#define ICON_MENU_ITEM_H
/*
 * Adapted from Haiku's ProcessController (src/apps/processcontroller/
 * IconMenuItem.{h,cpp}), Copyright 2000 Georges-Edouard Berenger, MIT.
 * Trimmed to what ChoiceToolItem needs (icon optional, no Reset()/purge).
 */
#include <interface/MenuItem.h>

class BBitmap;

/**
 * @class IconMenuItem
 *
 * @brief BMenuItem that draws an optional small bitmap before its label.
 * Does not own/delete the bitmap - caller keeps responsibility for it,
 * same as ToolItem's own icon parameter.
 */
class IconMenuItem : public BMenuItem
{
public:
							IconMenuItem(BBitmap *icon, const char *title,
								BMessage *message);
	virtual					~IconMenuItem(void) {};

	virtual	void			DrawContent(void);
	virtual	void			Highlight(bool isHighlighted);
	virtual	void			GetContentSize(float *width, float *height);

			/** not owned - see the class note. Used by ChoiceToolItem to
			 * repeat the marked choice's icon in the field itself. */
			BBitmap*		Icon(void) {return icon;};

protected:
			void			DrawIcon(void);

			BBitmap			*icon;
};
#endif
