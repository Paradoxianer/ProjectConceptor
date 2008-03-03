#ifndef BOOL_RENDERER_H
#define BOOL_RENDERER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <interface/Bitmap.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>

#include "NodeEditor.h"
#include "PDocument.h"
#include "PCommand.h"
#include "Renderer.h"
#include "Renderer.h"
#include "TextEditorControl.h"

#ifdef B_ZETA_VERSION
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif 


class BoolRenderer: public Renderer
{

public:
							BoolRenderer(NodeEditor *parentEditor,bool forValue,BRect valueRect,BMessage *message=NULL);
			void			ValueChanged(void){};
			void			Draw(BView *drawOn, BRect updateRect);
			void			SetBool(bool newValue);
			bool			GetBool(void){return value;};
			void			MouseDown(BPoint where);
			void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg){};

			void			MouseUp(BPoint where);
			void			SetFrame(BRect newRect);
			BRect			Frame(void){return frame;};
			void			MoveBy(float dx, float dy){frame.OffsetBy(dx,dy);};
			void			ResizeBy(float dx,float dy){frame.right+=dx;SetFrame(frame);};

			bool			Selected(void){return false;};
			bool			Caught(BPoint where){frame.Contains(where);};

protected:
			void			Init();
		BMessage			*changeMessage;
		NodeEditor 		*editor;
		BRect				frame;
		bool				value;
		BBitmap				*selected;
		BBitmap				*unselected;
private:
};
#endif
