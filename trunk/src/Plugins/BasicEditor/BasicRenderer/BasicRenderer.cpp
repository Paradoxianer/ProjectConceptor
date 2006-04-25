#include "BasicRenderer.h"
#include "BasicClassView.h"
#include "BasicConnectionView.h"
#include "BasicGroupView.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	BasicRenderer *basicDocument=new BasicRenderer( id );
  	return basicDocument;
}

BasicRenderer::BasicRenderer(image_id id):BasePlugin(id)
{
}

void* BasicRenderer::GetNewObject(void *value)
{
	BMessage	*theValue		= (BMessage *)value;
	void		*returnValue	= NULL;
	switch(theValue->what) 
	{
		case P_C_CLASS_TYPE:
			returnValue	= new BasicClassView(theValue);
			break;
		//this shoud be rendered by ClassView
/*		case P_C_FUNCTION_TYPE:
			returnValue	= new  BasicFunctionView(theValue);
			break;
		case P_C_MEMBER_TYPE:
			returnValue	= new BasicMemberView(theValue);
			break;*/
		case P_C_GROUP_TYPE:
			returnValue = new BasicGroupView(theValue);
		case P_C_CONNECTION_TYPE:
			returnValue	= new BasicConnectionView(theValue);
	}		
	return returnValue;
}
