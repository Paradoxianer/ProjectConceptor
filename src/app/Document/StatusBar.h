#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <interface/View.h>
#include <support/List.h>
#include <support/String.h>

#include <vector>

/** The window's status bar. On the left named texts that any editor can
 * set - e.g. "selection" or "pointer" - shown in the order they first
 * appeared; on the right views an editor adds, e.g. its zoom control.
 */
class StatusBar : public BView
{
public:
							StatusBar(BRect frame, const char *name);

			/** an empty text hides the slot but keeps its place; NULL
			 * removes it */
			void			SetText(const char *slot, const char *text);
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
	};
			void			_LayoutRightViews(void);

			std::vector<Slot>	fSlots;
			BList			fRightViews;
};

#endif
