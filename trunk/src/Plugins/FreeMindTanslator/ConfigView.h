#ifndef CONFIG_VIEW_H
#define CONFIG_VIEW_H

#include "SettingsManager.h"
#include <interface/View.h>

class ConfigView : public BView
{
public:
			ConfigView(void);

protected:
	SettingsManager	*settingsManager;
private:
};
#endif
