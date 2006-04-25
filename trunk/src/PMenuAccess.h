#ifndef PMENU_ACCESS_H
#define PMENU_ACCESS_H

#include <interface/MenuBar.h>
#include <interface/Menu.h>
#include <interface/MenuItem.h>

#include "ToolBar.h"
#include "ToolMenu.h"
#include "ToolItem.h"

/**
 * @class PMenuAccess
 * @brief PMenuAccess Provide acces interface for Menu and Toolbar
 * If you use this Interface you don´t need to take care of Loakalisation
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 *
 * @see PDocument
 * @see PWindow
 *
 * @see ToolBar
 * @see ToolMenu
 * @see ToolItem
 * @see BMenu
 * @see BMenuItem
 */

class PMenuAccess
{

public:
	virtual	BMenu*		GetMenu(const char* signatur) = 0;
	virtual	BMenuItem*	GetMenuItem(const char* signatur) = 0;

	virtual	ToolBar*	GetToolBar(const char *signatur) = 0;
	virtual	ToolMenu*	GetToolMenu(const char* toolbarSignature,const char *signature) = 0;
	virtual	ToolItem*	GetToolItem(const char* toolbarSignature,const char *signature) = 0;


	virtual	status_t	AddMenu(BMenu *menu,int32 index = -1) = 0;
	virtual	status_t	AddMenuItem(const char *menuSignatur, BMenuItem *menuItem,int32 index = -1) = 0;

	virtual	status_t	AddToolBar(ToolBar *toolbar) = 0;
	virtual	status_t	AddToolMenu(const char *toolbarSignatur,ToolMenu *toolMenu) = 0;
	virtual	status_t	AddToolItem(const char *toolbarSignatur,const char *toolmenuSignatur,ToolItem *toolItem,int32 index = -1) = 0;


	virtual status_t	RemoveMenu(const char* menuSignature) = 0;
	virtual status_t	RemoveMenuItem(const char* signature) = 0;

	virtual status_t	RemoveToolBar(const char* signature) = 0;
	virtual status_t	RemoveToolMenu(const char* toolbarSignature,const char* signature) = 0;
	virtual status_t	RemoveToolItem(const char* toolbarSignature,const char* signature) = 0;
private:
};
#endif
