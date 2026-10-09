#ifndef CONTEXT_BAR_H
#define CONTEXT_BAR_H

#include <Message.h>
#include <Rect.h>
#include <String.h>

#include <vector>

class BView;
struct GraphStyle;

/** A small row of buttons floating above the selected nodes (add an
 * attribute, group them). Works in view pixels, so it keeps its size at
 * any zoom; GraphEditor draws it last and asks it first on a click.
 */
class ContextBar
{
public:
							ContextBar(void);
							~ContextBar(void);

			/** label is shown as is; the bar owns message */
			void			AddButton(const char *label, BMessage *message);
			int32			CountButtons(void) const {return fButtons.size();};

			/** places the buttons, each width[i] wide (text only), centered
			 * above anchor - or below it if there is no room above -
			 * and kept inside visible; height is the text's */
			void			Layout(BRect anchor, BRect visible,
								const std::vector<float> &widths, float height);
			void			Hide(void) {fVisible = false;};
			bool			IsVisible(void) const {return fVisible;};
			BRect			Frame(void) const {return fFrame;};
			BRect			ButtonFrame(int32 index) const;

			/** index of the button under where, -1 for none */
			int32			ButtonAt(BPoint where) const;
			BMessage*		MessageAt(int32 index) const;
			const char*		LabelAt(int32 index) const;
			void			SetHighlight(int32 index) {fHighlight = index;};
			int32			Highlight(void) const {return fHighlight;};

			void			Draw(BView *view, const GraphStyle &style) const;

private:
	struct Button {
		BString		label;
		BMessage	*message;
		BRect		frame;
	};
			std::vector<Button>	fButtons;
			BRect				fFrame;
			bool				fVisible;
			int32				fHighlight;
};

#endif
