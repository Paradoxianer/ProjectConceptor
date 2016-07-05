#include "PluginItem.h"
#include <View.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include "PluginManager.h"


PluginItem::PluginItem(BasePlugin *newPlugin, PluginManager *pluginManager, uint32 level, bool expanded):BListItem(level,expanded) {
    plugin=newPlugin;
 //   icon=pluginManager->GetIcon(newPlugin);
}

void PluginItem::Update(BView *owner,const BFont *font){
    BListItem::Update(owner,font);
    font_height fontHeigth;
    owner->GetFontHeight(&fontHeigth);
    baseLine=fontHeigth.leading+fontHeigth.descent+fontHeigth.ascent;
}

void PluginItem::DrawItem(BView *owner,BRect bounds,bool complete){
    rgb_color color;
    if (IsSelected()) {
	color = ui_color(B_CONTROL_HIGHLIGHT_COLOR) ;
    }
    else {
	color = owner->ViewColor();
    }
    //Draw Background
    owner->SetHighColor(color);
    owner->FillRect(bounds);
	if (icon != NULL){
		BRect iconRect=BRect(bounds.left,bounds.top,bounds.Height(),bounds.bottom);
		owner->DrawBitmap(icon,iconRect);
    }
    color=ui_color(B_CONTROL_TEXT_COLOR);
    if (!IsEnabled()) {
	color = tint_color(color,B_DISABLED_MARK_TINT);
    }
    owner->SetHighColor(color);
    owner->MovePenTo(bounds.Height()+2,bounds.bottom-2);
    /**@todo truncate string if to long*/
    owner->DrawString(plugin->GetName());
}
