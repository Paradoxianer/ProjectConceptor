#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <interface/View.h>
#include <support/List.h>
#include <support/String.h>

#include <vector>

/** The window's status bar. On the left named slots that any editor can
 * set - a text like "selection" or "pointer", or a view like editable
 * fields - shown in the order they first appeared; on the right views an
 * editor adds, e.g. its zoom control.
 */
class StatusBar : public BView
{
public:
							StatusBar(BRect frame, const char *name);

			/** an empty text hides the slot but keeps its place; NULL
			 * removes it */
			void			SetText(const char *slot, const char *text);
			/** shows view in the slot, at its preferred size; NULL
			 * removes the slot and its view. The caller keeps the view. */
			void			SetView(const char *slot, BView *view);
			void			AddRightView(BView *view);
			void			RemoveRightView(BView *view);

	virtual	void			Draw(BRect updateRect);
	virtual	void			FrameResized(float width, float height);

	/** height for the current plain font */
	static	float			PreferredHeight(void);

private:
	struct Slot {
		BString		name;
		BString		text;
		BView		*view;
		float		x;
	};
			void			_LayoutRightViews(void);
			void			_LayoutSlots(void);
			bool			_Shown(const Slot &slot) const;

			std::vector<Slot>	fSlots;
			BList			fRightViews;
};

#endif
