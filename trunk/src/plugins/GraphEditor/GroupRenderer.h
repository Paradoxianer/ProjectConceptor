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
#if defined(__HAIKU__) && __GNUC__ > 3
#include <vector>
#include <iterator>
using namespace std;
#else
#include <cpp/vector.h>
#include <cpp/iterator.h>
#endif


#include "NodeEditor.h"
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

class GroupRenderer: public ClassRenderer
{

public:
							GroupRenderer(NodeEditor *parentEditor, BMessage *forContainer);
/*			void			MouseDown(BPoint where);
			void			MouseUp(BPoint where);
			void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);*/
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
/*			Renderer*		FindRenderer(BPoint where);
			Renderer*		FindNodeRenderer(BPoint where);
			Renderer*		FindConnectionRenderer(BPoint where);*/
			Renderer*		FindRenderer(BMessage *container);



			float			Scale(void){return scale;};
			BList*			RenderList(void){return renderer;};
	static	bool			DrawRenderer(void *arg,void *editor);
			void			RecalcFrame(void);

			void			BringToFront(Renderer *wichRenderer);
			void			SendToBack(Renderer *wichRenderer);
				//------Group Special Methods


protected:
				void		Init();
				void		InsertRenderObject(BMessage *node);
				
/*				bool		MoveAll(void *arg,float dx, float dy);
				bool		ResizeAll(void *arg,float dx, float dy);*/
				


		//++++++Group Special Methods
		BList				*allNodes;
		BList				*renderer;
		Renderer			*father;
		float				scale;
		//-----Group Special Methods

private:
};
#endif
