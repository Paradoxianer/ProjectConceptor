#include "RectItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

RectItem::RectItem(BRect newFrame, char *newLabel, BRect newValue, BMessage *message):	BControl(newFrame,"recter",newLabel,message,B_FOLLOW_LEFT_RIGHT,0)
{
	left		= new BTextControl(BRect(3,3,Bounds().Width(),10),"left",_T("left"),"",message);
	AddChild(left);
	top			= new BTextControl(BRect(3,left->Frame().bottom+3,Bounds().Width(),10),"top",_T("top"),"",message);
	AddChild(top);
	right		= new BTextControl(BRect(3,top->Frame().bottom+3,Bounds().Width(),10),"right",_T("right"),"",message);
	AddChild(right);
	bottom		= new BTextControl(BRect(3,right->Frame().bottom+3,Bounds().Width(),10),"bottom",_T("bottom"),"",message);
	AddChild(bottom);
	ResizeTo(Bounds().Width(),bottom->Frame().bottom+3);
	SetRect(newValue);
}

BRect RectItem::Rect(void)
{
	BRect	rect(atof(left->Text()),atof(top->Text()),atof(right->Text()),atof(bottom->Text()))	;
	return rect;
}

void RectItem::SetRect(BRect newValue)
{
	char*	svalue		= new char[24];
	sprintf(svalue,"%.2f",newValue.left);
	left->SetText(svalue);
	sprintf(svalue,"%.2f",newValue.top);
	top->SetText(svalue);
	sprintf(svalue,"%.2f",newValue.right);
	right->SetText(svalue);
	sprintf(svalue,"%.2f",newValue.bottom);
	bottom->SetText(svalue);
}
