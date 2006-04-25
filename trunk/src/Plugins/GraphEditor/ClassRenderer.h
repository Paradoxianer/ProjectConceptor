#ifndef BASIC_CLASS_VIEW_H
#define BASIC_CLASS_VIEW_H

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <interface/Box.h>
#include <interface/PictureButton.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>
#include "StringRenderer.h"

#include "GraphEditor.h"
#include "PDocument.h"
//#include "PCommand.h"
#include "Renderer.h"

#ifdef B_ZETA_VERSION
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif 

class ClassRenderer: public Renderer
{

public:
							ClassRenderer(GraphEditor *parentEditor,BMessage *forContainer);
		virtual void		Draw(BView *drawOn, BRect updateRect);
		virtual	void		MouseDown(BPoint where);
		virtual	void		MouseUp(BPoint where);
		virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg); 
		virtual	void		LanguageChanged();
		virtual	void		MessageReceived(BMessage *message);

		virtual void		ValueChanged(void);

		virtual bool		Caught(BPoint where);
		virtual BRect		Frame(void);

protected:
		virtual void		Init();
		BMessage			*viewMessage;
		static	bool		MoveAll(void *arg,void *deltaPoint);
		static	bool		ResizeAll(void *arg,void *deltaPoint);

	//++++++++++ClassSettings++++++++++
	float				xRadius,yRadius;
	rgb_color			fillColor,borderColor;
	BRect				frame;
	bool				selected;
	BFont				*font;
	float				penSize;
	
	//const char*			name;
	//---------ClassSettings-----------

	BPoint				*startMouseDown;
	BPoint				*startLeftTop;

	BPoint				*oldPt;

	int					connecting;
	bool				resizing;

	PDocument			*doc;
	BMessenger			*sentTo;
	
	BList				*outgoing;
	BList				*incoming;
	StringRenderer		*name;
	StringRenderer		*inhalt;
private:
};
#endif
