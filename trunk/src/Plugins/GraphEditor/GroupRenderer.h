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
#include <cpp/vector.h>
#include <cpp/iterator.h>


#include "GraphEditor.h"
#include "PDocument.h"
#include "Renderer.h"
#include "StringRenderer.h"
#include "PCommandManager.h"
#include "Renderer.h"
#include "ClassRenderer.h"
#include "ConnectionRenderer.h"
#include "GroupRenderer.h"

#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif

class GroupRenderer: public Renderer
{

public:
							GroupRenderer(GraphEditor *parentEditor, Renderer *parentRenderer, BMessage *forContainer);
			void			Draw(BView *drawOn, BRect updateRect);
			void			MouseDown(BPoint where);
			void			MouseUp(BPoint where);
			void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
			void			LanguageChanged();
			void			MessageReceived(BMessage *message);

			void			ValueChanged(void);

			bool			Caught(BPoint where);
			BRect			Frame(void);
			void			SetFrame(BRect newFrame);
			void			MoveBy(float dx, float dy);
			void			ResizeBy(float dx,float dy);
			bool			Selected(){return selected;};
			
				//++++++Group Special Methods
			void			AddRenderer(Renderer* newRenderer);
			void			RemoveRenderer(Renderer* wichRenderer);
			Renderer*		FindRenderer(BPoint where);
			Renderer*		FindNodeRenderer(BPoint where);
			Renderer*		FindConnectionRenderer(BPoint where);
			Renderer*		FindRenderer(BMessage *container);

			void			BringToFront(Renderer *wichRenderer);
			void			SendToBack(Renderer *wichRenderer);

			float			Scale(void){return scale;};
			BList*			RenderList(void){return renderer;};
	static	bool			DrawRenderer(void *arg,void *editor);

				//------Group Special Methods


protected:
				void		Init();
				void		InsertAttribute(char *attribName, BMessage *attribute,int32 count);
				void		InsertRenderObject(BMessage *node);
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

		StringRenderer		*name;

		//++++++Group Special Methods
		BList				*allNodes;
		BList				*allConnections;
		BList				*renderer;
		Renderer			*activRenderer;
		Renderer			*father;

		Renderer			*mouseReciver;
		float				scale;
		//-----Group Special Methods

private:
};
#endif
