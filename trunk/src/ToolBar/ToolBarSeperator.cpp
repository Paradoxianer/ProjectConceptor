#include "ToolBarSeperator.h"
#include "ToolBar.h"

ToolBarSeperator::ToolBarSeperator(menu_layout ori):BaseItem("TOOL_BAR_SEPERATOR")
{
	kind = 0;
	seperatormenu_layout=ori;
	if (ori == B_ITEMS_IN_ROW) 
		frame=BRect(0,0,TOOL_BAR_SEPERATOR_WIDTH,ITEM_HEIGHT);
	else	
		frame=BRect(0,0,ITEM_WIDTH,TOOL_BAR_SEPERATOR_WIDTH);
	
}
ToolBarSeperator::ToolBarSeperator(BMessage *archive):BaseItem(archive)
{
	status_t err;
	err = archive->FindInt32("ToolBarSeperator::kind",(int32 *) &kind); 
	//testen ob err OK und ob gültiges Frame...
	if (err != B_OK)
		kind = 0;
	err = archive->FindInt32("ToolBarSeperator::seperatormenu_layout",(int32 *) &seperatormenu_layout); 
	if (err !=  B_OK)
		seperatormenu_layout=B_ITEMS_IN_ROW;
}

ToolBarSeperator::~ToolBarSeperator(void)
{
}

void ToolBarSeperator::Draw(BRect /*updateRect*/)
{
	if (parentToolBar->GetLayout() == B_ITEMS_IN_ROW)
	{
	}
	else
	{
	}
}
status_t ToolBarSeperator::Archive(BMessage *archive,bool deep=true) const
{
	//**Errors ordentlich auswerten
	status_t err;
	err = BArchivable::Archive(archive, deep); 
	err = archive->AddString("class", "ToolBarSeperator"); 
	err = archive->AddInt32("ToolBarSeperator::kind",kind);
	err = archive->AddInt32("ToolBarSeperator::seperatormenu_layout",seperatormenu_layout);
	return err;
}

BArchivable* ToolBarSeperator::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "ToolBarSeperator") )
		return NULL;
	return new ToolBarSeperator(archive);
}

void ToolBarSeperator::SetLayout(menu_layout justification)
{
	if (seperatormenu_layout != justification)
	{
		float height = frame.Height();
		float width  = frame.Width();
		ResizeTo(height,width);
		seperatormenu_layout=justification;
	}
}

