#ifndef PROJEKT_CONCEPTOR_DEFS_H
#define PROJEKT_CONCEPTOR_DEFS_H
/*
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 */
#define _ZETA_USING_EXPERIMENTAL_
#include <app/AppDefs.h>
#include <support/TypeConstants.h>

#define DEBUG 1
#include <support/Debug.h>

//extern const Debugger		*myDebugger;
//		= new Debugger("ProjektConceptor");

//main type for Translation

#define	P_C_DOCUMENT_MIMETYPE	"application/x-vnd.projectconceptor-document"

#define P_C_DOCUMENT_TYPE 'pcDT'
#define P_C_DOCUMENT_RAW_TYPE 'pcRT'


const uint32	P_C_PLUGIN_MESSAGE				= 'pcPM';

const uint32	P_C_EDITOR_PLUGIN_TYPE			= 'pcET';
const uint32	P_C_COMMANDO_PLUGIN_TYPE		= 'pcCT';
const uint32	P_C_ITEM_PLUGIN_TYPE			= 'pcIT';
const uint32	P_C_ITEM_INPORT_EXPORT_TYPE		= 'pcTT';

const uint32	P_C_DATA_CELL_RENDERER			= 'pcDR';
const uint32	P_C_MESSAGE_FILTER				= 'pcMF';
const uint32	P_C_RENDER_FILTER				= 'pcRF';

const uint32	P_C_CLASS_TYPE					= 'pcOT';
const uint32	P_C_FUNCTION_TYPE				= 'pcFT';
const uint32	P_C_MEMBER_TYPE					= 'pcMT';
const uint32	P_C_GROUP_TYPE					= 'pcGT';
const uint32	P_C_CONNECTION_TYPE				= 'pcCT';
const uint32	P_C_MACRO_TYPE					= 'pcTM';

const uint32	P_C_INSERT_EDITOR				= 'pcIE';
const uint32	P_C_REMOVE_EDITOR				= 'pcRE';
/*const uint32	P_C_GROUP_TYPE					= 'pcGr';
const uint32	P_C_NODE_TYPE					= 'pcNd';
const uint32	P_C_FUNCTION_TYPE				= 'pcFc';
const uint32	P_C_STRUCT_TYPE					= 'pcSt';
const uint32	P_C_CONNECTION_TYPE				= 'pcCT';*/



/*const uint32	P_C_DELETE_OBJECT				= 'pcDO';
const uint32	P_C_INSERT_OBJECT				= 'pcDI';

const uint32	P_C_OBJECT_MOVED				= 'pcOM';
const uint32	P_C_CONNECTING					= 'pcCo';
const uint32	P_C_CONNECTED					= 'pcCd';*/

/**This type of Message used to execute Commands
 * every P_C_EXECUTE_COMMAND shoud contain a pointer to the PCommand object wich coud handle the Message
 * it can contain a Bool called shadow wich means, when it´s true, that this command is not recorded to the Undo and Macro List
 *
 * for more details of a special Command Protocoll please take a look at the explizit documentation for this Command
 *
 *@see PCommand
 */	

const uint32	P_C_EXECUTE_COMMAND				= 'pcEC';

const uint32	P_C_VALUE_CHANGED				= 'pcVC';
const uint32	P_C_DOC_BOUNDS_CHANGED			= 'pcDB';

const uint32	P_C_EDITOR_SWITCHED_ACTIV		= 'pcSA';
const uint32	P_C_EDITOR_SWITCHED_INACTIV		= 'pcSI';

const uint32	P_C_WINDOW_LIST_CHANGED			= 'pcWC';
const uint32	P_C_TOOLBAR_CHANGED				= 'pcTC';

#ifndef B_ZETA_VERSION_1_0_0

const uint32	B_REDO							= 'redo';
const uint32	B_CLEAR							= 'clea';
#endif

const uint32	WINDOW_REGISTRY_ADD				= 'WRad';
const uint32	WINDOW_REGISTRY_SUB				= 'WRsb';
const uint32	WINDOW_REGISTRY_ADDED			= 'WRdd';

// Messages for menu commands
const uint32	MENU_APP_ABOUT					= B_ABOUT_REQUESTED;
const uint32	MENU_APP_SETTINGS				= 'MEse';
const uint32	MENU_APP_HELP					= 'MEhe';
const uint32	MENU_APP_QUIT					= B_QUIT_REQUESTED;



const uint32	MENU_FILE_NEW					= 'MFnw';
const uint32	MENU_FILE_NEW_TAB				= 'MFnt';

const uint32	MENU_FILE_OPEN					= 'MFop';
const uint32	MENU_FILE_CLOSE					= 'MFcl';
const uint32	MENU_FILE_SAVE					= 'MFsv';
const uint32	MENU_FILE_SAVEAS				= 'MFsa';
const uint32	MENU_FILE_EXPORT				= 'MFex';
const uint32	MENU_FILE_PAGESETUP				= 'MFps';
const uint32	MENU_FILE_PRINT					= 'MFpr';
const uint32	MENU_FILE_QUIT					= 'MFqu';

const uint32	MENU_EDIT_UNDO					= 'MEun';
const uint32	MENU_EDIT_REDO					= 'MErd';

const uint32	MENU_EDIT_PROJECT_SETTINGS		= 'MEps';

const uint32	MENU_SEARCH_FIND				= 'MSfi';
const uint32	MENU_SEARCH_FIND_NEXT			= 'MSfn';
const uint32	MENU_SEARCH_REPLACE				= 'MSre';
const uint32	MENU_SEARCH_REPLACE_AND_FIND	= 'MSrf';
const uint32	MENU_SEARCH_REPLACE_ALL			= 'MSra';

const uint32	MENU_WINDOW_TITLE				= 'MWtl';
const uint32	MENU_WINDOW_TITLE_VERTICAL		= 'MWtv';
const uint32	MENU_WINDOW_CASCADE				= 'MWcs';

const uint32	MENU_MACRO_START_RECORDING		= 'MMsr';
const uint32	MENU_MACRO_STOP_RECORDING		= 'MMsp';
const uint32	MENU_MACRO_PLAY					= 'MMpl';
const uint32	MENU_MACRO_OPEN					= 'MMop';
const uint32	MENU_MACRO_SAVE					= 'MMsv';

const uint32	MENU_HELP_ABOUT					= 'MHab';


extern const char		*APP_SIGNATURE;//					= "application/ProjektConceptor";;
/**string with wich you can find the correspondenting BMenubar for entry´s like File, Edit and so on
 *@see BMenuBar
 */


extern const char*		P_MENU_APP_ABOUT;//				= "About";
extern const char*		P_MENU_APP_SETTINGS;//			= "Settings";
extern const char*		P_MENU_APP_HELP;//				= "Help";

extern const char*		P_MENU_APP_QUIT;//				= "Quit";

extern const char*		P_M_MENU_BAR;//					= "P_M_MENU_BAR";
/**string with wich you can find the correspondenting BMenubar wich is used for Status menus and information 
 *@see BMenuBar
 */
extern const char*		P_M_STATUS_BAR;//					= "P_M_STATUS_BAR";
/**string with wich you can find the correspondenting ToolBar for all this things like open, save... 
 *@see ToolBar
 */
extern const char*		P_M_STANDART_TOOL_BAR;//			= "P_M_STANDART_TOOL_BAR";
/**string with wich you can find the correspondenting ToolBar for all formating stuff 
 *@see ToolBar
 */
extern const char*		P_M_FORMAT_TOOL_BAR;//				= "P_M_FORMAT_TOOL_BAR";
/**string with wich you can find the correspondenting ToolBar for Editor related ToolItems
 *@see ToolBar
 */
extern const char*		P_M_EDITOR_TOOL_BAR;//				= "P_M_EDITOR_TOOL_BAR";


/**value to find the BMenu "File" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE;//						= "File";

/**value to find the BMenu Item "File->New" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE_NEW;//					= "New";
extern const char*		P_MENU_FILE_NEW_TAB;//				= "New Tab";
/**value to find the BMenu Item "File->Open" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE_OPEN;//				= "Open";
/**value to find the BMenu Item "File->Close" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE_CLOSE;//				= "Close";
/**value to find the BMenu Item "File->Save" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE_SAVE;//				= "Save";
/**value to find the BMenu Item "File->Save As" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE_SAVE_AS;//			= "Save As";
/**value to find the BMenu Item "File->PageSetup" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
 
extern const char*		P_MENU_FILE_EXPORT;//				= "Export";

extern const char*		P_MENU_FILE_PAGESETUP;//			= "Page Setup";
/**value to find the BMenu Item "File->Print" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE_PRINT;//				= "Print";
/**value to find the BMenu Item "File->Quit" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_FILE_QUIT;//				= "Quit";
/**value to find the BMenu "Edit" over the PMenuAcces Interface
 *@see PMenuAcces
 */	

extern const char*		P_MENU_EDIT;//					= "Edit";
extern const char*		P_MENU_EDIT_UNDO;//				= "Undo";
extern const char*		P_MENU_EDIT_REDO;//				= "Redo";

extern const char*		P_MENU_EDIT_CUT;//				= "Cut";
extern const char*		P_MENU_EDIT_COPY;//				= "Copy";
extern const char*		P_MENU_EDIT_PASTE;//				= "Paste";

extern const char*		P_MENU_EDIT_CLEAR;//				= "Clear";
extern const char*		P_MENU_EDIT_SELECT_ALL;//			= "Select All";

extern const char*		P_MENU_EDIT_PROJECT_SETTINGS;//	= "Project Settings";

extern const char*		P_MENU_SEARCH;//					= "Search";
extern const char*		P_MENU_SEARCH_FIND;//				= "Find";
extern const char*		P_MENU_SEARCH_FIND_NEXT;//			= "Find Next";
extern const char*		P_MENU_SEARCH_REPLACE;//			= "Replace";
extern const char*		P_MENU_SEARCH_REPLACE_AND_FIND;//	= "Replace & Find";
extern const char*		P_MENU_SEARCH_REPLACE_ALL;//		= "Replace All";

/**value to find the BMenu "Window" over the PMenuAcces Interface
 *@see PMenuAcces
 */	

extern const char*		P_MENU_WINDOW;//					= "Window";
extern const char*		P_MENU_WINDOW_TITLE;//				= "Title";
extern const char*		P_MENU_WINDOW_TITLE_VERTICAL;//	= "Title Vertical";
extern const char*		P_MENU_WINDOW_CASCADE;//			= "Cascade";


/**value to find the BMenu "Makrko" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
extern const char*		P_MENU_MACRO;//					= "Macro";
extern const char*		P_MENU_MACRO_START_RECORDING;//	= "Start Recording";
extern const char*		P_MENU_MACRO_STOP_RECORDING;//		= "Stop Recording";
extern const char*		P_MENU_MACRO_PLAY;//				= "Play";
extern const char*		P_MENU_MACRO_OPEN;//				= "Open";
extern const char*		P_MENU_MACRO_SAVE;//				= "Save";

extern const char*		P_MENU_HELP;//						= "Help";
extern const char*		P_MENU_HELP_ABOUT;//				= "About";


extern const char*		P_C_VERSION;//						= "0.01a Revision 132";


#endif

