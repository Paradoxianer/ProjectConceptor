#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
/*
 * @author Paradoxon powered by Jesus Christ
 * takes care about loading and saving the ConfigMessage for ProjectConceptor and the plugins
 */

#include <app/Message.h>
#include <storage/File.h>
#include <stdlib.h>

#include "ConfigWindow.h"
#include "MessageXmlReader.h"
#include "MessageXmlWriter.h"

/**
 * @class ProjektConceptor
 * @brief ConfigManager wich is responsible to create the Bridge for the settings between programm and file and the gui
 *
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.02
 * @date 2016/06/25
 *
 */

class ConfigManager
{
public:
						ConfigManager( char *path,BMessage* newConfig=NULL);
	/** returns the Configuration for the given name, or the whole config if
	 *  NULL. Passing a name returns a copy owned by the caller (delete it
	 *  when done, and pass an edited copy to SetConfigMessage() to persist
	 *  changes); passing NULL returns the live config object - do not
	 *  delete it.
	 */
	BMessage*           GetConfigMessage(const char *name=NULL);
	/** pass a komplete new Configration to the Manager causes it to rewrite the Config file and recreate a Config GUI
	 *  if you pass a NULL for the char it replaces the whole config Message
	 */
	status_t			SetConfigMessage(const char *name,BMessage *newConfig);
	bool				LoadConfig(void);
	void				SaveConfig();

private:
	void				_SetDefaults(void);
	BMessage			*config;
	BString				*path;
   // ConfigWindow		*configWindow;
    BMessenger			 *configMessenger;
};
#endif
