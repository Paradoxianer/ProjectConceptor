#ifndef PLUGININFOVIEW_H
#define PLUGININFOVIEW_H
#include <GridView.h>
#include "BasePlugin.h"
#include <map>
using namespace std;

#include <String.h>

class PluginInfoView : public BGridView
{
public:
    PluginInfoView(BasePlugin *_plugin);
    static map<uint32, BString>  	pcTypes;

};

#endif // PLUGININFOVIEW_H
