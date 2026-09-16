#include "LayoutPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	LayoutPlugin *layoutPlugin=new LayoutPlugin( id );
  	return layoutPlugin;
}

LayoutPlugin::LayoutPlugin(image_id id):BasePlugin(id)
{
}
