#include "PCSavePanel.h"
#include "ImportExport.h"
#include "BasePlugin.h"
#include <interface/Window.h>


PCSavePanel::PCSavePanel(PluginManager *pManager,BMessage *msg): BFilePanel(B_SAVE_PANEL,NULL,NULL,B_FILE_NODE,false,msg,NULL,false,true)
{
	pluginManager	= pManager;
	float height	= 74;
	Window()->SetTitle(_T("Save As"));
	Window()->Lock();
	BView *background = Window()->ChildAt(0),*dirmenufield;
	BRect limit 			= background->Bounds();
	limit.bottom			= height;
	BMenu	*format_menu	= BuildFormatsMenu();
	format_menu->ItemAt(0)->SetMarked(true);
	format_menu->SetLabelFromMarked(true);
	BRect rect;
	dirmenufield=Window()->FindView("DirMenuField");
	if(dirmenufield!=NULL)
	{
		dirmenufield->ResizeBy(-1*dirmenufield->Bounds().Width()/2,0);
		rect=dirmenufield->ConvertToParent(dirmenufield->Bounds());
		rect.OffsetBy(rect.Width()+5,0);
		rect.right=background->Bounds().right-10;
	}
	else
	{
		//use view coordinates to set base for drop-down menu
		BRect taille=background->Bounds();
		float x_center=taille.right-100;
		rect.Set(x_center,   taille.top+20, x_center+126,    taille.top+20+12  );
		rect.OffsetBy(-20,2);
	}

	formatMenu = new BMenuField(rect,"",NULL,format_menu,B_FOLLOW_TOP | B_FOLLOW_RIGHT ,B_WILL_DRAW);
	background->AddChild(formatMenu);
	Window()->Unlock();
}

BMenu *PCSavePanel::BuildFormatsMenu(void)
{
	// Builds a menu with all the possible file types
	BMessage *message;
	bool first=true;
	int32 typecode;
	BMenu *menu = new BMenu(_T("Fileformat") );
	BMenuItem *item;
	BList	*importExportPlugins	= pluginManager->GetPluginsByType(P_C_ITEM_INPORT_EXPORT_TYPE);
	if (importExportPlugins!=NULL)
	{
		ImportExport	*exporter;
		p_c_i_o_format	*formatStart;
		p_c_i_o_format	*format;
		int32			countFormat;
		for (int32 i = 0;i<importExportPlugins->CountItems();i++)	
		{
			exporter = (ImportExport *)((BasePlugin *)importExportPlugins)->GetNewObject(NULL);
			if (exporter)
			{
				exporter->GetOutputFormats((const p_c_i_o_format **)&formatStart,&countFormat);
				format = formatStart;
				for (int32 q = 0;q<countFormat;q++)
				{
					item = new BMenuItem(format->name, message);
					menu->AddItem( item );
					format++;
				}
				delete[] formatStart;
				formatStart = NULL;
				countFormat	= 0;
			}
		}
	}
	else
		menu->AddItem(new BMenuItem(_T("Could not find Plugins"),NULL));
	return menu;
}
