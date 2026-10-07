#include "ProjectConceptorDefs.h"

// Untranslated constants only: also compiled into the translators, which
// don't link libProjectConceptor.so so any app can load them.

const int32		P_C_VERSION			= 12;

const char*		P_C_DOC_FORMAT_VERSION_FIELD	= "pc:format_version";
const int32		P_C_DOC_FORMAT_VERSION			= 2;

const char*		P_C_DOC_AUTOSAVE_ENABLED		= "AutoSave::enabled";
const char*		P_C_DOC_AUTOSAVE_INTERVAL		= "AutoSave::interval";

const char*		P_C_CONFIG_SHORTCUTS_FIELD		= "Shortcuts";
const char*		P_C_SHORTCUT_ACTION_DELETE		= "Delete";
const char*		P_C_SHORTCUT_ACTION_ADD_BOOL	= "AddBool";
const char*		P_C_SHORTCUT_ACTION_INSERT_NODE	= "InsertNode";
const char*		P_C_SHORTCUT_KEY_FIELD			= "key";
const char*		P_C_SHORTCUT_MODIFIERS_FIELD	= "modifiers";

const char*		P_C_CONFIG_MACRO_SHORTCUTS_FIELD	= "MacroShortcuts";
const char*		P_C_MACRO_SHORTCUT_NAME_FIELD		= "macroName";

const char*		P_C_CONFIG_WINDOW_FRAME_FIELD		= "WindowFrame";
const char*		P_C_WINDOW_FRAME_RECT_FIELD		= "frame";
const char*		P_C_WINDOW_FRAME_REMEMBER_FIELD	= "remember";


/*used to construct all ProjectConceptor Nodes*/
const char*		P_C_NODE_NAME					= "Node::name";
const char*		P_C_NODE_DATA					= "Node::Data";
const char*		P_C_NODE_FONT					= "Node::Font";
const char*		P_C_NODE_FRAME					= "Node::Frame";

const char*		P_C_NODE_PATTERN				= "Node::Pattern";
const char*		P_C_NODE_SHAPE					= "Node::Shape";
const char*		P_C_SHAPE_NAME					= "Shape::name";
const char*		P_C_SHAPE_TEXT_RECT				= "Shape::textRect";
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
const char*		P_C_NODE_CONNECTION_ARROWS		= "Connection::arrows";

const char*		P_C_NODE_INCOMING				= "Node::incoming";
const char*		P_C_NODE_OUTGOING				= "Node::outgoing";
