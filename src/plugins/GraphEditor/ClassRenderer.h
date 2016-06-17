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


protected:
				void		Init();
				void		InsertAttribute(char *attribName, BMessage *attribute,int32 count);
				void		AdjustParents(BMessage* theParent, BMessage *command);

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
