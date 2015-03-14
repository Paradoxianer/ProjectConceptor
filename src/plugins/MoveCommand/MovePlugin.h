#ifndef MOVE_PLUGIN_H
#define MOVE_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Move.h"

class MovePlugin : public BasePlugin
{
public:

						MovePlugin(image_id id);
	
	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Move";};
	virtual char*			GetDescription(void){return "This is a simple Move Command for ProjectConceptor with basic funktionality";};
	virtual void*			GetNewObject(void *value){return new Move();};
};
#endif
