#ifndef CLASS_RENDERER_H
#define CLASS_RENDERER_H

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <interface/Box.h>
#include <interface/PictureButton.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>

//using the ugly stl instead of the nice Zeta templates to make it Haiku ready
#if defined(__HAIKU__) && __GNUC__ > 3
#include <vector>
#include <iterator>
using namespace std;
#else
#include <cpp/vector.h>
#include <cpp/iterator.h>
#endif

#include "GraphEditor.h"
#include "PDocument.h"
#include "Renderer.h"
#include "StringRenderer.h"
#include "AFont.h"

#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif

class ClassRenderer: public Renderer
{

public:
							ClassRenderer(GraphEditor *parentEditor, BMessage *forContainer);
				void		Draw(BView *drawOn, BRect updateRect);
				void		MouseDown(BPoint where);
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

protected:
				void		Init();
				void		InsertAttribute(char *attribName, BMessage *attribute,int32 count);
				void		AdjustParents(BMessage* theParent, BMessage *command);
		BMessage			*viewMessage;


		virtual	bool		MoveAll(void *arg,float dx, float dy);
		virtual	bool		ResizeAll(void *arg, float dx, float dy);

	//++++++++++ClassSettings++++++++++
		float				xRadius,yRadius;
		rgb_color			fillColor,borderColor;
		BRect				frame;
		bool				selected;
		AFont				*font;
		float				penSize;


	//---------ClassSettings-----------
		BMessage			*parentNode;

		BPoint				*startMouseDown;
		BPoint				*startLeftTop;

		BPoint				*oldPt;

		int					connecting;
		bool				resizing;

		PDocument			*doc;
		BMessenger			*sentTo;

		StringRenderer		*name;
		vector<Renderer*>	*attributes;

private:
};
#endif
