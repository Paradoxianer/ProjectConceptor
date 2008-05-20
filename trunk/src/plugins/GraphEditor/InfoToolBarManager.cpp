#include "InfoToolBarManager.h"


InfoToolBarManager::InfoToolBarManager(BList *_selected, BHandler *_target){
	FloatToolItem	*penSize;
}

void InfoToolBarManager::ValueChanged(void){
	BRect		rect;
	BMessage	*tmpNode;
	if (selected->CountItems() == 1){
		tmpNode	= (BMessage*)selected->ItemAt(0);
		switch(node->what){
			case P_C_CLASS_TYPE:
				newRenderer	= new  ClassRenderer(this,node);
			break;
			case P_C_GROUP_TYPE:
				newRenderer = new GroupRenderer(this,node);
			break;
			case P_C_CONNECTION_TYPE:
			break;
		}

	}
	else if (selected->CountItems() > 1){
		bool		widthFit	= true;
		bool		heightFit	= true;
		bool		topFit		= true;
		bool		leftFit		= true;
		rgb_color	marked		= {150;150;150;255};
		rgb_color	normal		= ;
		BRect	lastRect;
		tmpNode	= (BMessage*)selected->ItemAt(i);
		err = tmpNode->FindRect("Node::frame",&lastRect);
		for (int32 i=1; i<selected->CountItems();i++){
			tmpNode	= (BMessage*)selected->ItemAt(i);
			err = tmpNode->FindRect("Node::frame",&rect);
			if ((leftFit) && (rect.x != lastRect.x))
				leftFit = false;
			if ((topFit) && (rect.x != lastRect.x))
				topFit	= false;
			if ((widthFit) && (rect.Width() != lastRect.Width()))
				widthFit	= false;
			if ((heightFit) && (rect.Height() != lastRect.Height()))
				heightFit	= false;
			lastRect=rect;
		}
		left->SetValue(rect.left);
		top->SetValue(rect.top);
		width->SetValue(rect.Width());
		height->SetValue(rect.Height());
		if (leftFit)
			left->SetTextViewColor(marked);
		else
			left->SetTextViewColor(normal);

		if (topFit)
			top->SetTextViewColor(marked);
		else
			top->SetTextViewColor(normal);

		if (widthFit)
			width->SetTextViewColor(marked);
		else
			widtht->SetTextViewColor(normal);

		if (heightFit)
			height->SetTextViewColor(marked);
		else
			height->SetTextViewColor(normal);
	}
}


void InfoToolBarManager::Init(void){
	infoToolBar = new ToolBar();
	left		= FloatToolItem();
	top			= FloatToolItem();
	width		= FloatToolItem();
	height		= FloatToolItem();
	infoTollBar->AddItem(left);
	infoTollBar->AddItem(top);
	infoTollBar->AddItem(width);
	infoTollBar->AddItem(height);
}

void InfoToolBarManager::ClassToolBar(BMessage *node){
	status_t err = B_OK;
	err = node->FindRect("Node::frame",&rect);
	if (err == B_OK) {
		left->SetValue(rect.left);
		top->SetValue(rect.top);
		width->SetValue(rect.Width());
		height->SetValue(rect.Height());
	}
}

void InfoToolBarManager::ConnectionToolBar(BMessage *node){

}

void InfoToolBarManager::GroupToolBar(BMessage *node){
	ClassToolBar(node);
}
