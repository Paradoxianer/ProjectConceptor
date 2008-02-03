#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H
#include <OS.h>

class SettingsManager
{
public:
				SettingsManager(const char * str = NULL);
				~SettingsManager();
	void		SetSaveUndo(bool newValue){saveUndo = newValue;};
	bool		SaveUndo(void){return saveUndo;};
	void		SetSaveMacro(bool newValue){saveMacro = newValue;};
	bool		SaveMacro(void){return saveMacro;};
	void		SetRestoreWinState(bool newValue){restoreWinState = newValue;};
	bool		RestoreWinState(void){return restoreWinState;};

private:
	bool		saveUndo;
	bool		saveMacro;
	bool		restoreWinState;
	bool		changed;
};
#endif
