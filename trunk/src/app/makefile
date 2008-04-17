## BeOS Generic Makefile v2.0 ##

## Fill in this file to specify the project being created, and the referenced
## makefile-engine will do all of the hard work for you.  This handles both
## Intel and PowerPC builds of the BeOS.

## Application Specific Settings ---------------------------------------------

# specify the name of the binary
NAME= ProjectConceptor

# specify the type of binary
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel Driver
TYPE= APP

#	specify the source files to use
#	full paths or paths relative to the makefile can be included
# 	all files, regardless of directory, will have their object
#	files created in the common object directory.
#	Note that this means this makefile will not work correctly
#	if two source files with the same name (source.c or source.cpp)
#	are included from different directories.  Also note that spaces
#	in folder names do not work well with this makefile.
SRCS= 	About/AboutWindow.cpp \
		About/AboutView.cpp \
		About/AboutURLView.cpp \
		MainView.cpp \
		ProjectConceptor.cpp \
		Config/ConfigWindow.cpp \
		Config/ConfigView.cpp \
		Config/BoolItem.cpp \
		Config/ColorItem.cpp \
		Config/FloatItem.cpp \
		Config/FontItem.cpp \
		Config/MessageView.cpp \
		Config/PatternItem.cpp \
		Config/RectItem.cpp \
		Config/MessageItem.cpp \
		Config/StringItem.cpp \
		Commands/PCommand.cpp \
		Commands/PCommandManager.cpp \
		Document/PagedView.cpp \
		Document/PDocument.cpp \
		Document/PDocumentManager.cpp \
		Document/PEditor.cpp \
		Document/PEditorManager.cpp \
		Document/PWindow.cpp \
		Document/HelpManager.cpp \
		plugins/BasePlugin.cpp \
		plugins/PluginManager.cpp \
		ToolBar/BaseItem.cpp \
		ToolBar/Mover.cpp \
		ToolBar/ToolBar.cpp \
		ToolBar/ToolBarSeperator.cpp \
		ToolBar/ToolItem.cpp \
		ToolBar/ToolMenu.cpp\
		ToolBar/ColorToolItem.cpp \
		ToolBar/FloatToolItem.cpp \
		ToolBar/PatternToolItem.cpp \
		Tools/AFont.cpp \
		Tools/BViewSplitter.cpp \
		Tools/Divider.cpp \
		Tools/PCSavePanel.cpp \
		Tools/PDocLoader.cpp \
		Tools/Indexer.cpp \
		Tools/InputRequestView.cpp \
 		Tools/InputRequest.cpp  \
 		Tools/ShortCutFilter.cpp \
		Tools/TextViewCompleter.cpp



#	specify the resource files to use
#	full path or a relative path to the resource file can be used.
RSRCS= 	ProjectConceptor.rsrc \
		Tools/InputRequest.rsrc

#	specify additional libraries to link against
#	there are two acceptable forms of library specifications
#	-	if your library follows the naming pattern of:
#		libXXX.so or libXXX.a you can simply specify XXX
#		library: libbe.so entry: be
#		
#	- 	if your library does not follow the standard library
#		naming scheme you need to specify the path to the library
#		and it's name
#		library: my_lib.a entry: my_lib.a or path/my_lib.a
LIBS= be root translation tracker
		
#	specify additional paths to directories following the standard
#	libXXX.so or libXXX.a naming scheme.  You can specify full paths
#	or paths relative to the makefile.  The paths included may not
#	be recursive, so include all of the paths where libraries can
#	be found.  Directories where source files are found are
#	automatically included.
LIBPATHS= 

#	additional paths to look for system headers
#	thes use the form: #include <header>
#	source file directories are NOT auto-included here
SYSTEM_INCLUDE_PATHS = 

#	additional paths to look for local headers
#	thes use the form: #include "header"
#	source file directories are automatically included
LOCAL_INCLUDE_PATHS =	. \
					../include \
					../include/plugins \
					Commands \
					Config \
					DataContainer \
					Document \
					ToolBar \
					About \
					Tools

#	specify the level of optimization that you desire
#	NONE, SOME, FULL
OPTIMIZE= FULL

#	specify any preprocessor symbols to be defined.  The symbols
#	will be set to a value of 1.  For example specify DEBUG if you want
#	DEBUG=1 to be set when compiling.
DEFINES =   DEBUG

#	specify special warning levels
#	if unspecified default warnings will be used
#	NONE = supress all warnings
#	ALL = enable all warnings
WARNINGS = NONE

#	specify whether image symbols will be created
#	so that stack crawls in the debugger are meaningful
#	if TRUE symbols will be created
SYMBOLS = TRUE

#	specify debug settings
#	if TRUE will allow application to be run from
#	a source-level debugger
DEBUGGER = TRUE

#	specify additional compiler flags for all files
COMPILER_FLAGS =

#	specify additional linker flags
LINKER_FLAGS =

SVG_ICON = ../../Data/Icons/Logogross.svg

#EXTRA_DEPS = copy

TARGET_DIR	=  .
OBJ_DIR		= obj.x86

INSTALL_DIR	= /boot/apps/ProjectConceptor
PLUGIN_DIR	= Plugins


APP_MENU	= Software

## include the makefile-engine
ifndef BUILDHOME
BUILDHOME = /boot/develop
endif
include $(BUILDHOME)/etc/makefile-engine


default .DEFAULT:

	mkdir -p ../../bin
	cp -rf $(NAME) ../../bin
	ln -sf $(NAME) ../../bin/ProjectConceptor.a




