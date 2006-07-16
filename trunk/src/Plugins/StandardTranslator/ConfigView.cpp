#include "ConfigView.h"

ConfigView::ConfigView(void): BView(BRect(0,0,100,100),"Test",B_FOLLOW_ALL_SIDES,0)
{
	settingsManager= new SettingsManager("ProjectConceptorTranslator_settings");
}
