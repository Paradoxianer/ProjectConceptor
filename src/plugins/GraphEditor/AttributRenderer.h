#ifndef ATTRIBUT_RENDERER_H
#define ATTRIBUT_RENDERER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <app/Messenger.h>

#include <interface/Bitmap.h>
#include <interface/Menu.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include "GraphEditor.h"
#include "PDocument.h"
#include "PCommand.h"
#include "Renderer.h"
#include "StringRenderer.h"
#include "TextEditorControl.h"


const float		DELETER_WIDTH		= 16;
class AttributRenderer: public Renderer
{

public:
							AttributRenderer(GraphEditor *parentEditor,BMessage *forAttribut,BRect attribRect,BMessage *chgMessage=NULL, BMessage *delMessage=NULL);
			void			ValueChanged(void){};
			void			Draw(BView *drawOn, BRect updateRect);
			void			SetAttribute(BMessage *newAttribut);
			BMessage*		GetAttribute(void){return attribut;};
			void			MouseDown(BPoint where,int32 buttons =0,
	                                  int32 clicks =0,int32 modifiers =0);
			void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg){};

			void			MouseUp(BPoint where);
			void			SetFrame(BRect newRect);
			BRect			Frame(void){return frame;};
			void			MoveBy(float dx, float dy);
			void			ResizeBy(float dx,float dy){frame.right+=dx;SetFrame(frame);};

			bool			Selected(void){return false;};
			bool			Caught(BPoint where){return frame.Contains(where);};

protected:
			void			Init();
		BMessage			*changeMessage;
		BMessage			*attribut;
		BMessage			*deleteMessage;
		GraphEditor 		*editor;
		BRect				frame;
		BRect				delRect;
		StringRenderer		*name;
		Renderer			*value;
		//Renderer			*deleter;
		float				divider;
		BPopUpMenu			*kontextMenu;
		BBitmap				*delBitmap;
		
private:
};
#endif
