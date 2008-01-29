#include <string.h>
#include <interface/Alert.h>
#include <storage/Resources.h>
#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif 

#include "PluginManager.h"
#include "BasePlugin.h"

PluginManager::PluginManager(void)
{
	plugins		= new BList();
	groupNames	= new BList();
	
}

PluginManager::~PluginManager(void)
{
/*	BasePlugin *currentPlugin	= NULL;
	for (int32 i = 0; i<plugins->CountItems();i++)
	{
		currentPlugin	= (BasePlugin *)plugins->ItemAt(i);
		unload_add_on(currentPlugin->GetPluginID());
	}
	delete	plugins;
	delete	groupNames;*/
}

void PluginManager::LoadPlugins(BDirectory *startDir, bool deep)
{
	TRACE();
	BList		*directoryList	= new BList();
	BList		*dirNames		= new BList();
	BEntry 		*entry			= new BEntry(); 
	int32		dirCounter		= 0;
	BDirectory	*dir			= NULL;
	image_id	addonId;
	BPath		path(".");
	BasePlugin* (*NewProjektConceptorPlugin)(image_id);
	directoryList->AddItem(startDir);
	dirNames->AddItem((void *)".");
	while( directoryList->CountItems()>dirCounter)
	{
		dir=(BDirectory *)directoryList->ItemAt(dirCounter);
		while( (dir != NULL) && (dir->GetNextEntry(entry, TRUE ) == B_OK) )
		{
			if ((entry->InitCheck() == B_OK) && (entry->GetPath(&path) == B_OK))
			{
				//Versuch die über path angegebene Datei als Addon zu laden
//				DE_BUG("PluginManager","Plugin   %s\n",path.Path());
				addonId = load_add_on( path.Path() );
				if( addonId < 0 )
				{
					//Wenns schief ging, kein Poblem, dann wars irgendeine andere Datei
					if ((entry->IsDirectory())&&deep)
					{
						directoryList->AddItem(new BDirectory(entry));
						dirNames->AddItem((void *)path.Leaf());
					}
				}
				else
				{
					PRINT(("\t%s\t\tloaded\n", path.Leaf()));
					/*überprüfen ob es ein Addon war, welches unsere Schnittstelle entspricht
					Schnittstelle heist hier NewImmersiveVideoPlugin und es muss mit dem Protypen
					(void **)NewImmersiveVideoPlugin gekennzeichnet*/
					if( get_image_symbol( addonId, 
										  "NewProjektConceptorPlugin", 
										   B_SYMBOL_TYPE_TEXT, 
										   (void **)&NewProjektConceptorPlugin) )
					{
						// wenn wir ein ungülitges Plugin geladen hatten, wieder wegwerfen ;-)
						PRINT(("DEBUG:\tget_image_symbol( NewImmersiveVideoPlugin ) failed\n"));
						unload_add_on(addonId );
					}
					else
					{
						BasePlugin *aktPlugin = (*NewProjektConceptorPlugin)(addonId);
						if( !aktPlugin)
						{
							PRINT(("ERROR:\tPluginManager LoadPlugins\n failed to create a new plugin\n"));
						}
						else
						{
							//plugins->Insert(make_pair((BString *)dirNames->ItemAt(dirCounter),aktPlugin));
						//	plugins[(BString *)dirNames->ItemAt(dirCounter)] = aktPlugin;
							plugins->AddItem(aktPlugin);
							groupNames->AddItem(dirNames->ItemAt(dirCounter));
//							container->AddPointer("plugins",(const void *)aktPlugin);
						}
					}
				}
			}
		}
		dirCounter++;
	}
	if (plugins->CountItems()<1)
	  (new BAlert(_T("ProjectConceptor"), _T("Cant find Plugins"), "OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
}


BList *PluginManager::GetPluginsByType(uint32 type)
{
	BList		*tmpPlugins = new BList();
	BasePlugin	*aktPlugin	= NULL;

	for (int i=0;i<plugins->CountItems();i++)
	{
		aktPlugin=(BasePlugin *)plugins->ItemAt(i);
		if (aktPlugin->GetType()==type)
			tmpPlugins->AddItem(aktPlugin);
	}
	if (tmpPlugins->CountItems()==0)
		return NULL;
	else
		return tmpPlugins;
//	return GetPluginsByKindAndType(P_C_ANY_KIND,type);
}

BList *PluginManager::GetPluginsByGroup(const char *groupName)
{
	BList		*tmpPlugins = new BList();
	char		*grName;

	for (int i=0;i<plugins->CountItems();i++)
	{
		grName=(char *)groupNames->ItemAt(i);
		if (strcmp(groupName,grName)==B_OK)
			tmpPlugins->AddItem(plugins->ItemAt(i));
	}
	if (tmpPlugins->CountItems()==0)
		return NULL;
	else
		return tmpPlugins;

}
BList *PluginManager::GetPluginsByGroupAndType(const char *groupName, uint32 type)
{
	BList		*tmpPlugins = new BList();
	BasePlugin	*aktPlugin	= NULL;
	char		*grName;

	for (int i=0;i<plugins->CountItems();i++)
	{
		aktPlugin	= (BasePlugin *)plugins->ItemAt(i);
		grName		= (char *)groupNames->ItemAt(i);
		if ((aktPlugin->GetType()==type) && (strcmp(groupName,grName)==B_OK))
			tmpPlugins->AddItem(aktPlugin);
	}
	if (tmpPlugins->CountItems()==0)
		return NULL;
	else
		return tmpPlugins;
//	return GetPluginsByKindAndType(P_C_ANY_KIND,type);

}
BBitmap *PluginManager::GetIcon(BasePlugin *plugin)
{
	BBitmap	*bmp	= NULL;
	image_info	*info=new image_info;
	size_t	size;
	const void	*data;
	get_image_info(plugin->GetPluginID(),info);
	BResources *res=new BResources(new BFile((const char*)info->name,B_READ_ONLY));
	if (bmp==NULL)
		bmp=new BBitmap(BRect(0.0, 0.0, 15.0, 15.0),B_CMAP8);
	data=res->LoadResource((type_code)'MICN',"BEOS:M:STD_ICON",&size);
	if (data)
	{
		bmp->Lock();
		bmp->SetBits(data,size,0,B_CMAP8);
		bmp->Unlock();
	}
	return bmp;
}
