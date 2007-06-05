#include "ConnectionRenderer.h"
#include "ClassRenderer.h"
#include <interface/Window.h>
#include <stdio.h>


ConnectionRenderer::ConnectionRenderer(GraphEditor *parentEditor, BMessage *forContainer):Renderer(parentEditor, forContainer)
{
	TRACE();
	Init();
}
void ConnectionRenderer::Init()
{	
	TRACE();
	from			= NULL;
	to				= NULL;
	fromPoint		= BPoint(0,0);
	toPoint			= BPoint(0,0);
	selected		= false;
	fillColor		= make_color(187,67,47,255);
//	connectionName	= new BTextControl(BRect(0,0,100,55),"Name",NULL,"Unbenannt",new BMessage(B_C_NAME_CHANGED));
//	AddChild(connectionName);
	BList		*outgoing	= NULL;
	BList		*incoming	= NULL;
	BMessage	*fromNode	= NULL;
	BMessage	*toNode		= NULL;
	BMessage	*data		= new BMessage();
	container->FindPointer("Node::from",(void **)&fromNode);
	container->FindPointer("Node::to",(void **)&toNode);
	PRINT_OBJECT(*fromNode);
	PRINT_OBJECT(*toNode);
	if (fromNode->FindPointer("Node::outgoing",(void **)&outgoing) != B_OK)
	{
		outgoing = new BList();
		fromNode->AddPointer("Node::outgoing",outgoing);
	}
	if (!outgoing->HasItem(container))
		outgoing->AddItem(container);
	if (toNode->FindPointer("Node::incoming",(void **)&incoming) != B_OK)
	{
		incoming = new BList();
		toNode->AddPointer("Node::incoming",incoming);
	}
	if (!incoming->HasItem(container))
		incoming->AddItem(container);
	if (container->FindMessage("Node::Data",data) != B_OK)
	{
		data->AddString("Name","Unbenannt");
		container->AddMessage("Node::Data",data);
	}
	container->FindPointer("ProjectConceptor::doc",(void **)&doc);
	sentTo						= new BMessenger(NULL,doc);
//	PCommandManager	*commandManager	= doc->GetCommandManager();
//	selectCommand	= commandManager->GetPCommand("Select");
	PRINT_OBJECT(*container);
	ValueChanged();
}
	
void ConnectionRenderer::MouseDown(BPoint where)
{
	uint32 buttons = 0;
	BMessage *currentMsg = editor->Window()->CurrentMessage();
	currentMsg->FindInt32("buttons", (int32 *)&buttons);
	uint32 modifiers = 0;
	currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
	float newy	= where.x*ax+mx;
	float newx	= where.y*ay+my;
	float dx	= (newx-where.x);
	float dy	= (newy-where.y);
	if ((dx*dx+dy*dy)<max_entfernung)
	{
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
			editor->BringToFront(this);
		else if (buttons & B_SECONDARY_MOUSE_BUTTON )
			editor->SendToBack(this);
		if  (!selected) 
		{
			BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
			if  ((modifiers & B_SHIFT_KEY) != 0) 
				selectMessage->AddBool("deselect",false);
			selectMessage->AddPointer("node",container);
			selectMessage->AddString("Command::Name","Select");
			sentTo->SendMessage(selectMessage);
		}
	}
}
void ConnectionRenderer::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
}
void ConnectionRenderer::MouseUp(BPoint where)
{
}

void ConnectionRenderer::LanguageChanged()
{
}

void ConnectionRenderer::Draw(BView *drawOn, BRect updateRect)
{	
	//ValueChanged();
	CalcLine();
	drawOn->SetPenSize(2.0);
	BPoint	shadowFrom = fromPoint;
	BPoint	shadowTo = toPoint;
	BPoint	shadowfirst		= first;
	BPoint	shadowsecond	= second;
	BPoint	shadowthird		= third;
	shadowFrom.y	+=3;
	shadowTo.y		+=3;
	shadowfirst.y	+=3;
	shadowsecond.y	+=3;
	shadowthird.y	+=3;

	drawOn->SetHighColor(0,0,0,77);
	drawOn->StrokeLine(	shadowFrom,shadowTo);
	drawOn->FillTriangle(shadowfirst,shadowsecond,shadowthird);
	if (!selected)
		drawOn->SetHighColor(fillColor);
	else
		drawOn->SetHighColor(tint_color(fillColor,1.5));
	drawOn->StrokeLine(	fromPoint,toPoint);
	drawOn->FillTriangle(first,second,third);
}
void ConnectionRenderer::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case P_C_VALUE_CHANGED:
				ValueChanged();	
			break;
		case B_C_NAME_CHANGED:
		{
			BMessage	*data= new BMessage();
			container->FindMessage("Node::Data",data);
//			data->ReplaceString("Name",connectionName->Text());
			container->ReplaceMessage("Node::Data",data);
			break;
		}
	}
}

void ConnectionRenderer::ValueChanged()
{
	BMessage	*tmpNode	= NULL;
	container->FindPointer("Node::from",(void **)&tmpNode);
	tmpNode->FindPointer(editor->RenderString(),(void **)&from);
	tmpNode->PrintToStream();
	container->FindPointer("Node::to",(void **)&tmpNode);
	tmpNode->FindPointer(editor->RenderString(),(void **)&to);
	tmpNode->PrintToStream();
	container->FindBool("Node::selected",&selected);
}

void ConnectionRenderer::CalcLine()
{
	BRect	*fromRect	= new BRect(from->Frame());
	BRect	*toRect		= new BRect(to->Frame());
	float		toMiddleX 	=	(toRect->right-toRect->left)/2;
	float		toMiddleY	=	(toRect->bottom-toRect->top)/2;
	alpha		= atan2((toRect->top-fromRect->top),(toRect->left-fromRect->left));
	if ( (alpha < -M_PI_3_4 ) || (alpha > M_PI_3_4) )
	{
		first		= BPoint(toRect->right+triangleHeight,toRect->top+toMiddleY-triangleHeight);
		second		= BPoint(first.x,toRect->top+toMiddleY+triangleHeight);
		third		= BPoint(toRect->right,toRect->top+toMiddleY);
		toPoint		= BPoint(first.x,third.y);
	//	fromPoint	= BPoint(fromRect->left,fromRect->top+(fromRect->bottom-fromRect->top)/2);
	}
	else if (alpha < -M_PI_4) 
	{
		first		= BPoint(toRect->left+toMiddleX-triangleHeight,toRect->bottom+triangleHeight);
		second		= BPoint(toRect->left+toMiddleX+triangleHeight,toRect->bottom+triangleHeight);
		third		= BPoint(toRect->left+toMiddleX,toRect->bottom);
		toPoint		= BPoint(third.x,first.y);
	//	fromPoint	= BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->top);		
	}
	else if (alpha> M_PI_4)
	{
		first		= BPoint(toRect->left+toMiddleX-triangleHeight,toRect->top-triangleHeight);
		second		= BPoint(toRect->left+toMiddleX+triangleHeight,toRect->top-triangleHeight);
		third		= BPoint(toRect->left+toMiddleX,toRect->top);
		toPoint		= BPoint(third.x,first.y);	
	//	fromPoint	= BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->bottom);		
	}
	else
	{
		first		= BPoint(toRect->left-triangleHeight,toRect->top+toMiddleY-triangleHeight);
		second		= BPoint(toRect->left-triangleHeight,toRect->top+toMiddleY+triangleHeight);
		third		= BPoint(toRect->left,toRect->top+toMiddleY);
		toPoint		= BPoint(first.x,third.y);
		//fromPoint	= BPoint(fromRect->right,fromRect->top+(fromRect->bottom-fromRect->top)/2);
	}
	fromPoint	= toPoint;
	fromPoint.ConstrainTo(*fromRect);
	ax			= (toPoint.y-fromPoint.y)/(toPoint.x-fromPoint.x);
	mx			= fromPoint.y-(fromPoint.x*ax);
	ay			= (toPoint.x-fromPoint.x)/(toPoint.y-fromPoint.y);
	my			= fromPoint.x-(fromPoint.y*ay);

}


BRect ConnectionRenderer::Frame()
{
	float	left	= fromPoint.x;
	float	top		= fromPoint.y;
	float	right	= toPoint.x;	
	float	bottom	= toPoint.y;
	float	c;
	if (left>right)
	{
		c		= right;
		right	= left;
		left	= c;
	}
	if (top>bottom)
	{
		c		= top;
		top		= bottom;
		bottom	= c;
	}
	return BRect(left,top,right,bottom);
}
bool ConnectionRenderer::Caught(BPoint where)
{
	float newy	= (double)where.x*ax+mx;
	float newx	= (double)where.y*ay+my;
	float dx	= (newx-where.x);
	float dy	= (newy-where.y);
	if ((dx*dx+dy*dy)<max_entfernung)
		return true;
	else
		return false;
/*	float newx	= (where.x-fromPoint.x)*cos(alpha)+fromPoint.x;
	float newy	= (where.y-fromPoint.y)*sin(alpha)+fromPoint.y;
	printf("newx=%0.3lf\tnewy=%0.3lf\n",newx,newy);
	float dx	= newx-where.x;
	float dy	= newy-where.y;
	printf("dx=%0.3lf\tdy=%0.3lf\n",dx,dy);
	if ((dx*dx+dy*dy)<max_entfernung)
		return true;
	else
		return false;*/
}
