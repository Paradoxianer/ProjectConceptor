#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
/*
 * @author Paradoxon powered by Jesus Christ
 * takes care about loading and saving the ConfigMessage for ProjectConceptor and the plugins
 */

#include <app/Message.h>
#include <storage/File.h>

#include <stdlib.h>

class ConfigManager
{
public:
						ConfigManager(BMessage newConfig,, char *path);
	BMessage	GetConfigMessage(void){return config;};
	void				SetConfigMessage(BMessage *newConfig);
	void				LoadDefaults(void);
	void				SaveConfig(void);

private:
	TiXmlElement	ProcessMessage(BMessage *node);

	BMessage	*config
	BFile				*file;

};
#endif