#include "ProjectConceptorDefs.h"
#include <Catalog.h>



#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ProjectConceptor"


const int32		P_C_VERSION			= 12;



const char*		P_MENU_APP_HELP					= B_TRANSLATE("Help");
const char*		P_MENU_APP_ABOUT				= B_TRANSLATE("About");
const char*		P_MENU_APP_SETTINGS				= B_TRANSLATE("Settings");
const char*		P_MENU_APP_QUIT					= B_TRANSLATE("Quit");



const char*		P_M_MENU_BAR					= "P_M_MENU_BAR";
/**string with wich you can find the correspondenting BMenubar wich is used for Status menus and information
 *@see BMenuBar
 */
const char*		P_M_STATUS_BAR					= "P_M_STATUS_BAR";
/**string with wich you can find the correspondenting ToolBar for all this things like open, save...
 *@see ToolBar
 */
const char*		P_M_STANDART_TOOL_BAR			= "P_M_STANDART_TOOL_BAR";
/**string with wich you can find the correspondenting ToolBar for all formating stuff
 *@see ToolBar
 */
const char*		P_M_FORMAT_TOOL_BAR				= "P_M_FORMAT_TOOL_BAR";
/**string with wich you can find the correspondenting ToolBar for Editor related ToolItems
 *@see ToolBar
 */
const char*		P_M_EDITOR_TOOL_BAR				= "P_M_EDITOR_TOOL_BAR";


/**value to find the BMenu "File" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE						= B_TRANSLATE("File");

/**value to find the BMenu Item "File->New" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE_NEW					= B_TRANSLATE("New");
const char*		P_MENU_FILE_NEW_TAB				= B_TRANSLATE("New Tab");
/**value to find the BMenu Item "File->Open" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE_OPEN				= B_TRANSLATE("Open");
/**value to find the BMenu Item "File->Close" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE_CLOSE				= B_TRANSLATE("Close");
/**value to find the BMenu Item "File->Save" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE_SAVE				= B_TRANSLATE("Save");
/**value to find the BMenu Item "File->Save As" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE_SAVE_AS				= B_TRANSLATE("Save as");
/**value to find the BMenu Item "File->PageSetup" over the PMenuAcces Interface
 *@see PMenuAcces
 */

const char*		P_MENU_FILE_EXPORT				= B_TRANSLATE("Export");

const char*		P_MENU_FILE_PAGESETUP			= B_TRANSLATE("Page setup");
/**value to find the BMenu Item "File->Print" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE_PRINT				= B_TRANSLATE("Print");
/**value to find the BMenu Item "File->Quit" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_FILE_QUIT				= B_TRANSLATE("Quit");
/**value to find the BMenu "Edit" over the PMenuAcces Interface
 *@see PMenuAcces
 */

const char*		P_MENU_EDIT						= B_TRANSLATE("Edit");
const char*		P_MENU_EDIT_UNDO				= B_TRANSLATE("Undo");
const char*		P_MENU_EDIT_REDO				= B_TRANSLATE("Redo");

const char*		P_MENU_EDIT_CUT					= B_TRANSLATE("Cut");
const char*		P_MENU_EDIT_COPY				= B_TRANSLATE("Copy");
const char*		P_MENU_EDIT_PASTE				= B_TRANSLATE("Paste");

const char*		P_MENU_EDIT_CLEAR				= B_TRANSLATE("Clear");
const char*		P_MENU_EDIT_SELECT_ALL			= B_TRANSLATE("Select all");

const char*		P_MENU_EDIT_PROJECT_SETTINGS	= B_TRANSLATE("Settings");

const char*		P_MENU_SEARCH					= B_TRANSLATE("Search");
const char*		P_MENU_SEARCH_FIND				= B_TRANSLATE("Find");
const char*		P_MENU_SEARCH_FIND_NEXT			= B_TRANSLATE("Find next");
const char*		P_MENU_SEARCH_REPLACE			= B_TRANSLATE("Replace");
const char*		P_MENU_SEARCH_REPLACE_AND_FIND	= B_TRANSLATE("Find & Replace");
const char*		P_MENU_SEARCH_REPLACE_ALL		= B_TRANSLATE("Replace all");

/**value to find the BMenu "Window" over the PMenuAcces Interface
 *@see PMenuAcces
 */

const char*		P_MENU_WINDOW					= B_TRANSLATE("Window");
const char*		P_MENU_WINDOW_TITLE				= B_TRANSLATE("Tile");
const char*		P_MENU_WINDOW_TITLE_VERTICAL	= B_TRANSLATE("Tile Vertical");
const char*		P_MENU_WINDOW_CASCADE			= B_TRANSLATE("Cascade");


/**value to find the BMenu "Makrko" over the PMenuAcces Interface
 *@see PMenuAcces
 */
const char*		P_MENU_MACRO					= B_TRANSLATE("Macro");
const char*		P_MENU_MACRO_START_RECORDING	= B_TRANSLATE("Start recording");
const char*		P_MENU_MACRO_STOP_RECORDING		= B_TRANSLATE("Stop recording");
const char*		P_MENU_MACRO_PLAY				= B_TRANSLATE("Play");
const char*		P_MENU_MACRO_OPEN				= B_TRANSLATE("Open");
const char*		P_MENU_MACRO_SAVE				= B_TRANSLATE("Save Macro");

const char*		P_MENU_HELP						= B_TRANSLATE("Help");
const char*		P_MENU_HELP_ABOUT				= B_TRANSLATE("About");


/*used to construct all ProjectConceptor Nodes*/
const char*		P_C_NODE_NAME					= "Node::name";
const char*		P_C_NODE_DATA					= "Node::Data";
const char*		P_C_NODE_FONT					= "Node::Font";
const char*		P_C_NODE_FRAME					= "Node::Frame";

const char*		P_C_NODE_PATTERN				= "Node::Pattern";
const char*		P_C_NODE_SELECTED				= "Node::selected";
const char*		P_C_NODE_X_RADIUS				= "Node::xRadius";
const char*		P_C_NODE_Y_RADIUS				= "Node::yRadius";

const char*		P_C_NODE_CONNECTION_FROM		= "Node::from";
const char*		P_C_NODE_CONNECTION_TO			= "Node::to";

const char*		P_C_NODE_CREATED				= "Node::created";
const char*		P_C_NODE_MODIFIED				= "Node::modified";
const char*		P_C_NODE_PARENT					= "Node::parent";
const char*		P_C_NODE_ALLNODES				= "Node::allNodes";
const char*		P_C_NODE_ALLCONNECTIONS			= "Node::allConnections";

const char*		P_C_NODE_CONNECTION_TYPE		= "Connection::type";

const char*		P_C_NODE_INCOMING				= "Node::incoming";
const char*		P_C_NODE_OUTGOING				= "Node::outgoing";
