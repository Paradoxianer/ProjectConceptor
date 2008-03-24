#ifndef BASE_PLUGIN_H
#define BASE_PLUGIN_H

#include <kernel/image.h>
#include <support/TypeConstants.h>
/**
 * @class BasePlugin
 *
 * @brief  BasePlugin ist the parent of all supported Plugins
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 */
class BasePlugin
{

public:
						BasePlugin(image_id id);
			image_id	GetPluginID(void){return pluginID;};
//	virtual void		LanguageChanged(void)=0;
	/**
	 * returns the Name of the Plugin .. in the case  of P_C_COMMANDO_PLUGIN_TYPE
	 * this name ist used in PCommandManager::GetPCommand to find the right PCommand
	 */
	virtual	char*		GetName(void)				= 0;
	/**
	 * returns Autor of the Plugin
	 */
	virtual	char*		GetAutor(void)				= 0;
	/**
	 * returns a Versionsstring
	 */
	virtual char*		GetVersionsString(void)		= 0;
	/**
	 * returns a Description
	 */
	virtual char*		GetDescription(void)		= 0;
	/**
	 *	returns the kind of pluginType e.g.: 
	 * P_C_EDITOR_PLUGIN_TYPE means this plugins "contains" a PEditorPlugin
 	 * P_C_COMMANDO_PLUGIN_TYPE means this plugins "contains" a PCommandPlugin
 	 * you also can create your own pluginType (please dont use pc at the beginning this 
 	 * is used for all ProjectConcpetor consts
	 */
	virtual uint32		GetType(void)				= 0;
	/**
	 * GetNewObject returns a new instance (in case of P_C_EDITOR_PLUGIN_TYPE a new
	 * instance of a PEditor) you have to cast it for yourself to the right Class
	 */
	virtual void*		GetNewObject(void *value)	= 0;
	
	

protected:
		image_id	pluginID;
	private:
};
#endif
