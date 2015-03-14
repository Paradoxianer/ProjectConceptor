#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <app/Application.h>
#include <app/Message.h>
#include <kernel/image.h>
#include <storage/Directory.h>

#include <interface/Bitmap.h>

#include <storage/Entry.h>
#include <storage/Path.h>

#include "ProjectConceptorDefs.h"

class BasePlugin;

/**
 * @class PluginManager
 *
 * @brief  PluginManager is the responsibel to load all Plugins
 * and return a choice  of plugins
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.02
 * @date 2009/07/12
 *

 */

class PluginManager
{


public:
					PluginManager(void);
                                        ~PluginManager(void);
                void		LoadPlugins(BDirectory *startDir, bool deep=true);
                /**
                 * return all Plugins in one List;
                 */
                BList		*GetAllPlugins(void){return plugins;};
		/**
		 * return all Plugins with one special typ 
		 */
		BList		*GetPluginsByType(uint32 type);
		/**
		 * return all Plugins with one groupName.. groupName means that this pluings lie in a directory with this name
		 */
		BList		*GetPluginsByGroup(const char *groupName);
		BList		*GetPluginsByGroupAndType(const char *groupName,uint32 type);
		/**
		 * return the icon of this Plugin.. usefull ich you want to creat a ToolBarItem or something like this
		 * from the Plugin Icon 
		 */
		BBitmap		*GetIcon(BasePlugin *plugin);		

private:
		BList		*plugins;
		BList		*groupNames;
		
};
#endif
