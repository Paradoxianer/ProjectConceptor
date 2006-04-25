#include "RectItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

RectItem::RectItem(char *newLabel, BRect newValue, BMessage *message):ConfigItem(B_RECT_TYPE),BBox(BRect(0,0,100,100),"recter")
{
	label		= newLabel;
	left		= new BTextControl(BRect(3,3,Bounds().Width(),10),"left",_T("left"),"",message);
	AddChild(left);
	top			= new BTextControl(BRect(3,left->Frame().bottom+3,Bounds().Width(),10),"top",_T("top"),"",message);
	AddChild(top);
	right		= new BTextControl(BRect(3,top->Frame().bottom+3,Bounds().Width(),10),"right",_T("right"),"",message);
	AddChild(right);
	bottom		= new BTextControl(BRect(3,right->Frame().bottom+3,Bounds().Width(),10),"bottom",_T("bottom"),"",message);
	AddChild(bottom);
	ResizeTo(Bounds().Width(),bottom->Frame().bottom+3);
	SetValue(&newValue);
}

void* RectItem::GetValue(void)
{
	BRect	rect(atof(left->Text()),atof(top->Text()),atof(right->Text()),atof(bottom->Text()))	;
	//SetValue((void *)&rect);
	return &rect;
}

void RectItem::SetValue(void* newValue)
{
	BRect	rect		= *(BRect *)(newValue);
	char*	svalue		= new char[24];
	sprintf(svalue,"%.2f",rect.left);
	left->SetText(svalue);
	sprintf(svalue,"%.2f",rect.top);
	top->SetText(svalue);
	sprintf(svalue,"%.2f",rect.right);
	right->SetText(svalue);
	sprintf(svalue,"%.2f",rect.bottom);
	bottom->SetText(svalue);
}
