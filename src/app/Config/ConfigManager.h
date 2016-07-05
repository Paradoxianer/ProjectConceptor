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
	/** returns a Pointer to the BMessage holding the Configuration for the given name
	 *  if the name is NULL all goes to the main settings)
	 */
	BMessage*           GetConfigMessage(const char *name=NULL);
	/** pass a komplete new Configration to the Manager causes it to rewrite the Config file and recreate a Config GUI
	 *  if you pass a NULL for the char it replaces the whole config Message
	 */
	status_t			SetConfigMessage(const char *name,BMessage *newConfig);
	void				LoadConfig(void);
	void				SaveConfig();

private:
	void				TestInit(void);
	BMessage			*config;
	BString				*path;
   // ConfigWindow		*configWindow;
    BMessenger			 *configMessenger;
};
#endif
