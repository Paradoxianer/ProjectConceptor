#ifndef GROUP_RENDERER_H
#define GROUP_RENDERER_H

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <interface/Box.h>
#include <interface/PictureButton.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>

//using the ugly stl instead of the nice Zeta templates to make it Haiku ready
#include <vector>
#include <iterator>
using namespace std;


#include "GraphEditor.h"
#include "PDocument.h"
#include "Renderer.h"
#include "StringRenderer.h"
#include "PCommandManager.h"
#include "Renderer.h"
#include "ClassRenderer.h"
#include "ConnectionRenderer.h"
#include "GroupRenderer.h"


class GroupRenderer: public ClassRenderer
{

public:
							GroupRenderer(GraphEditor *parentEditor, BMessage *forContainer);
			void			MouseDown(BPoint where, int32 buttons =0,
	                              int32 clicks =0,int32 modifiers =0);
/*			void			MouseUp(BPoint where);
			void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);*/
			/** Renders as a skyline-style boundary that hugs each
			 * child's own rect (plus margin) column by column, not one
			 * bounding-box rect - a short child stays short even when a
			 * taller one sits in a neighbouring column. Gaps with no
			 * child at all just hold the last height, keeping the shape
			 * one connected piece (issue #38). frame/P_C_NODE_FRAME
			 * (RecalcFrame()) stay a plain bounding rect regardless -
			 * only Draw()'s own shape changes, hit-testing/serialization
			 * are unaffected.
			 */
			void			Draw(BView *drawOn, BRect updateRect);
			void			LanguageChanged();
//			void			MessageReceived(BMessage *message);

			void			ValueChanged(void);

/*			bool			Caught(BPoint where);
			BRect			Frame(void);
			void			SetFrame(BRect newFrame);*/
			void			MoveBy(float dx, float dy);
			void			ResizeBy(float dx,float dy);
//			bool			Selected(){return selected;};
			
				//++++++Group Special Methods
			void			AddRenderer(Renderer* newRenderer);
			void			RemoveRenderer(Renderer* wichRenderer);
			Renderer*		FindRenderer(BMessage *container);



			float			Scale(void){return scale;};
			BList*			RenderList(void){return renderer;};
	static	bool			DrawRenderer(void *arg,void *editor);
			void			RecalcFrame(bool toFit=true);

			void			BringToFront(Renderer *wichRenderer);
			void			SendToBack(Renderer *wichRenderer);
				//------Group Special Methods

			/** no manual resize handle - a group's box is always an
			 * auto-fit rectangle around its children (issue #38). */
	virtual	bool			SupportsResize(void) {return false;};


protected:
				void		Init();
				void		InsertRenderObject(BMessage *node);
				/** each child's own rect plus the margin the outline keeps
				 * around it - shared by Draw() and PlaceLabel() so both
				 * agree on where the shape actually sits. */
				void		CollectChildRects(vector<BRect> &rects);
				/** height kept clear above the leftmost child for this
				 * group's name and attribute rows. */
				float		LabelSpace(void);
				/** moves the name/attributes into the outline's own label
				 * notch - ClassRenderer places them against `frame`, which
				 * for a group is the whole children's bounding box (#38). */
				void		PlaceLabel(void);
				
/*				bool		MoveAll(void *arg,float dx, float dy);
				bool		ResizeAll(void *arg,float dx, float dy);*/
				


		//++++++Group Special Methods
		BList				*allNodes;
		BList				*renderer;
		Renderer			*father;
		float				scale;
		/** true while this group still carries the editor's standard fill
		 * colour, i.e. was never given one of its own - it then draws a
		 * faint tint and no drop shadow (#38). */
		bool				usesDefaultFill;
		//-----Group Special Methods

private:
};
#endif
