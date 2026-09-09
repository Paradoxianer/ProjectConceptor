#ifndef CLASS_RENDERER_H
#define CLASS_RENDERER_H

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <interface/Box.h>
#include <interface/PictureButton.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>

#include <vector>
#include <iterator>
using namespace std;

#include "GraphEditor.h"
#include "PDocument.h"
#include "Renderer.h"
#include "StringRenderer.h"
#include "AFont.h"


/**
 * @class ClassRenderer
 * @brief Renders a node for the GrapEditorView
 *
 * @author Paradoxon 
 * @todo improve @ClassRenderer::ValueChanged() Method to not create all Attributerender news
 * @bug the name of Attributerenderes are change to be the same when the class get resized
 */



class ClassRenderer: public Renderer
{

public:
							ClassRenderer(GraphEditor *parentEditor, BMessage *forContainer);
				void		Draw(BView *drawOn, BRect updateRect);
				void		MouseDown(BPoint where, int32 buttons =0,
	                              int32 clicks =0,int32 modifiers =0);
				void		MouseUp(BPoint where);
				void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
				void		LanguageChanged();
				void		MessageReceived(BMessage *message);

				void		ValueChanged(void);

				bool		Caught(BPoint where);
				BRect		Frame(void);
				void		SetFrame(BRect newFrame);
				void		MoveBy(float dx, float dy);
				void		ResizeBy(float dx,float dy);
				bool		Selected(void){return selected;};
				BMessage*	Parent(void){return parentNode;};
				/** puts the name label straight into edit mode - used right
				 * after a fresh insert so the user can type a name
				 * immediately, without simulating it on every construction
				 * (that used to also fire on a plain document load, see #69)
				 */
				void		StartEditingName(void){name->MouseDown(BPoint(0,0));};

				void		SetPreviewFillColor(rgb_color color);
				void		ClearPreviewFillColor(void);

				bool		AnimationStep(float dt);

				/** false hides the resize handle/hit-test entirely (issue
				 * #38 - a group's box is auto-fit around its children, a
				 * manual resize handle would just get overridden by
				 * RecalcFrame() the moment it commits anyway). True here,
				 * overridden in GroupRenderer.
				 */
		virtual	bool		SupportsResize(void) {return true;};


protected:
				void		Init();
				void		InsertAttribute(char *attribName, BMessage *attribute,int32 count);
				void		AdjustParents(BMessage* theParent, BMessage *command);

		virtual	bool		MoveAll(void *arg,float dx, float dy);
		virtual	bool		ResizeAll(void *arg, float dx, float dy);

	//++++++++++ClassSettings++++++++++
		float				xRadius,yRadius;
		rgb_color			fillColor,borderColor;
		bool				hasPreviewFillColor;
		rgb_color			previewFillColor;
		BRect				frame;
		bool				selected;

		/** spring-animates the drawn position toward frame.LeftTop() after
		 * a non-interactive move (e.g. Auto-Layout) - see ValueChanged()/
		 * AnimationStep(). Draw() offsets by (animPosX,animPosY)-frame.
		 * LeftTop() via BView::SetOrigin(), frame/children stay at their
		 * real, final position throughout - same "renderer draws something
		 * other than the committed data, briefly" pattern as
		 * SetPreviewFillColor() (see docs/notes.md).
		 */
		bool				animating;
		float				animPosX,animPosY;
		float				animVelX,animVelY;
		/** false until the constructor's first ValueChanged() call has run -
		 * skips animating a brand new node in from BRect(0,0,0,0). */
		bool				initialized;
		AFont				*font;
		float				penSize;


	//---------ClassSettings-----------
		BMessage			*parentNode;

		BPoint				*startMouseDown;
		BRect				*startFrame;

		BPoint				*oldPt;

		int32				connecting;
		bool				showConnecter;
		bool				resizing;

		PDocument			*doc;
		BMessenger			*sentTo;

		StringRenderer		*name;
		vector<Renderer*>	*attributes;
		BRect				leftConnection;
		BRect				topConnection;
		BRect				rightConnection;
		BRect				bottomConnection;
				

private:
};
#endif
