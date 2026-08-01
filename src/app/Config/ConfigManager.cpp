#include "ConfigManager.h"
#include <stdio.h>
#include <app/Messenger.h>
#include <interface/InterfaceDefs.h>
#include <storage/Entry.h>
#include <storage/Path.h>
#include <support/String.h>

#include <support/Debug.h>
#include <map>
#include <Catalog.h>

#include "MessageXmlReader.h"
#include "MessageXmlWriter.h"
#include "ProjectConceptorDefs.h"



ConfigManager::ConfigManager(char *_path, BMessage *newConfig){
	path = new BString(_path);
	if (newConfig != NULL) {
		config = newConfig;
		return;
	}
	// if no Message was passed we start from defaults, then try to load
	// the persisted config over them; only if no config file exists yet
	// do we persist the defaults.
	config = new BMessage();
	_SetDefaults();
	if (!LoadConfig())
		SaveConfig();
}

BMessage* ConfigManager::GetConfigMessage(const char *name){
	if (name == NULL)
		return config;
	// nested BMessages are stored flattened, not as live pointers, so this
	// always has to hand back a copy - modify it and pass it to
	// SetConfigMessage() to persist changes.
	BMessage	*subMessage	= new BMessage();
	if (config->FindMessage(name,subMessage) == B_OK)
		return subMessage;
	delete subMessage;
	return NULL;
}


status_t ConfigManager::SetConfigMessage(const char *name,BMessage *newConfig){
	status_t err	= B_OK;
	if (name == NULL) {
		delete config;
		config =newConfig;
	}
	else{
		err = config->ReplaceMessage(name,newConfig);
		if (err == B_NAME_NOT_FOUND || err ==B_BAD_INDEX) {
			err = config->AddMessage(name,newConfig);
		}
	}
	return err;
}

bool ConfigManager::LoadConfig(void){
	TRACE();
	MessageXmlReader messageXml = MessageXmlReader();
	messageXml.SetTo(*path);
	if (messageXml.InitCheck() != B_OK)
		return false;
	BMessage *loaded = messageXml.Read();
	if (loaded == NULL)
		return false;
	delete config;
	config = loaded;
	DEBUG_ONLY(config->PrintToStream());
	return true;
}

void ConfigManager::SaveConfig(){
	TRACE();
	DEBUG_ONLY(config->PrintToStream());
	MessageXmlWriter messageXml = MessageXmlWriter();
	messageXml.SetTo(*path);
	if (messageXml.InitCheck())
        messageXml.Write(*config);
}

void ConfigManager::_SetDefaults(){
	BMessage	shortcuts;
	BMessage	deleteBinding;
	deleteBinding.AddInt32(P_C_SHORTCUT_KEY_FIELD,B_DELETE);
	deleteBinding.AddInt32(P_C_SHORTCUT_MODIFIERS_FIELD,0);
	shortcuts.AddMessage(P_C_SHORTCUT_ACTION_DELETE,&deleteBinding);

	BMessage	addBoolBinding;
	addBoolBinding.AddInt32(P_C_SHORTCUT_KEY_FIELD,'b');
	addBoolBinding.AddInt32(P_C_SHORTCUT_MODIFIERS_FIELD,B_COMMAND_KEY);
	shortcuts.AddMessage(P_C_SHORTCUT_ACTION_ADD_BOOL,&addBoolBinding);

	BMessage	insertNodeBinding;
	insertNodeBinding.AddInt32(P_C_SHORTCUT_KEY_FIELD,B_INSERT);
	insertNodeBinding.AddInt32(P_C_SHORTCUT_MODIFIERS_FIELD,0);
	shortcuts.AddMessage(P_C_SHORTCUT_ACTION_INSERT_NODE,&insertNodeBinding);

	config->AddMessage(P_C_CONFIG_SHORTCUTS_FIELD,&shortcuts);

	// empty on first run - populated by the user via the Shortcuts settings
	// tab, entries are {key, modifiers, macroName} matched by index like
	// ShortCutFilter::AddShortCutList() expects.
	config->AddMessage(P_C_CONFIG_MACRO_SHORTCUTS_FIELD,new BMessage());
}
