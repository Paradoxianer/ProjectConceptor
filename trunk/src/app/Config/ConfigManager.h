#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
/*
 * @author Paradoxon powered by Jesus Christ
 * takes care about loading and saving the ConfigMessage for ProjectConceptor and the plugins
 */

#include <app/Message.h>
#include <storage/File.h>

#include <stdlib.h>

#include "tinyxml/tinyxml.h"

/**
 * @class ProjektConceptor
 * @brief ConfigManager wich is responsible to create the Bridge for the settings between programm and file and the gui
 *
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2009/10/03
 *
 */
class ConfigManager
{
public:
						ConfigManager( char *path,BMessage* newConfig=NULL);
	/** returns a Pointer to the BMessage holding the Configuration
	*/
	BMessage*	GetConfigMessage(void){return config;};
	/** pass a komplete new Configration to the Manager causes it to rewrite the Config file and recreate a Config GUI
	*/
	void				SetConfigMessage(BMessage *newConfig);
	void				LoadDefaults(void);
	void				SaveConfig();

private:
	TiXmlElement	ProcessMessage(BMessage *node);
	BMessage*		ProcessXML(TiXmlElement);

	BMessage	*config;
	BFile				*file;

};
#endif
