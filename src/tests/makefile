## BeOS Generic Makefile v2.0 ##
## CppUnit test binary for core classes (Indexer, PCommand) - issue #68.
## Recompiles the small dependency subset those classes need (mirrors the
## main app's SRCS list minus ProjectConceptor.cpp, which has its own
## main() and the ProjektConceptor BApplication subclass this doesn't
## need) rather than trying to link against the app executable the way
## add-ons do - a standalone test binary needs everything resolved at
## link time, add-ons resolve into the host process at load time instead.

NAME= ProjectConceptorTests

TYPE= APP

APP_MIME_SIG = application/x-vnd.ProjectConceptorTests

SRCS= 	TestMain.cpp \
		TestDocument.cpp \
		IndexerTest.cpp \
		PCommandTest.cpp \
		LayoutEditorTest.cpp \
		../plugins/LayoutEditor/LayoutEditor.cpp \
		../plugins/LayoutEditor/DotLayouter.cpp \
		../plugins/BatchCommand/Batch.cpp \
		../app/ProjectConceptorDefs.cpp \
		../app/Config/ConfigManager.cpp \
		../app/Config/ConfigWindow.cpp \
		../app/Config/ConfigView.cpp \
		../app/Config/ShortCutView.cpp \
		../app/Config/KeyCaptureWindow.cpp \
		../app/Config/PluginView.cpp \
		../app/Config/PluginItem.cpp \
		../app/Config/PluginInfoView.cpp \
		../app/Config/MessageView.cpp \
		../app/Commands/PCommand.cpp \
		../app/Commands/PCommandManager.cpp \
		../app/Document/PDocument.cpp \
		../app/Document/PDocumentManager.cpp \
		../app/Document/PDocLoader.cpp \
		../app/Document/PEditor.cpp \
		../app/Document/PEditorManager.cpp \
		../app/Document/PWindow.cpp \
		../app/Document/HelpManager.cpp \
		../app/MainView.cpp \
		../app/ToolBar/BaseItem.cpp \
		../app/ToolBar/Mover.cpp \
		../app/ToolBar/ToolBar.cpp \
		../app/ToolBar/ToolBarSeperator.cpp \
		../app/ToolBar/ToolItem.cpp \
		../app/ToolBar/ToolMenu.cpp \
		../app/ToolBar/ColorToolItem.cpp \
		../app/ToolBar/ColorSwatchView.cpp \
		../app/ToolBar/AlphaSlider.cpp \
		../app/ToolBar/ColorPickerWindow.cpp \
		../app/ToolBar/FloatToolItem.cpp \
		../app/ToolBar/ChoiceToolItem.cpp \
		../app/ToolBar/IconMenuItem.cpp \
		../app/ToolBar/PatternToolItem.cpp \
		../app/Tools/AFont.cpp \
		../app/Tools/PCSavePanel.cpp \
		../app/Tools/Indexer.cpp \
		../app/Tools/InputRequestView.cpp \
		../app/Tools/InputRequest.cpp \
		../app/Tools/MessageXmlReader.cpp \
		../app/Tools/MessageXmlWriter.cpp \
		../app/Tools/ShortCutFilter.cpp \
		../app/Tools/TextViewCompleter.cpp \
		../app/Tools/plugins/BasePlugin.cpp \
		../app/Tools/plugins/PluginManager.cpp \
		../plugins/ChangeValueCommand/ChangeValue.cpp \
		../plugins/InsertCommand/Insert.cpp \
		../plugins/GroupCommand/Group.cpp

RDEFS=

RSRCS=

LIBS= be root translation tracker localestub cppunit $(STDCPPLIBS) tinyxml

LIBPATHS=

SYSTEM_INCLUDE_PATHS =	/boot/system/develop/headers/private/support \
						/boot/system/develop/headers/private/shared \
						/boot/system/develop/headers/private/interface

LOCAL_INCLUDE_PATHS =	. \
						../include \
						../include/plugins \
						../app \
						../app/Commands \
						../app/Config \
						../app/Document \
						../app/ToolBar \
						../app/About \
						../app/Tools \
						../plugins/ChangeValueCommand \
						../plugins/InsertCommand \
						../plugins/GroupCommand \
						../plugins/LayoutEditor \
						../plugins/BatchCommand

COMPILER_FLAGS =

LINKER_FLAGS =

TARGET_DIR	=  .

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
