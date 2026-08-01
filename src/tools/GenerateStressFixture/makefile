## BeOS Generic Makefile v2.0 ##
## Dev-only tool: builds a large (250+ node) stress-test .pcd fixture by
## constructing a real headless PDocument and calling its actual Archive()/
## Flatten() path - see GenerateStressFixture.cpp and docs/notes.md. Same
## dependency-subset approach as src/tests/makefile, minus CppUnit/PCommand.

NAME= GenerateStressFixture

TYPE= APP

APP_MIME_SIG = application/x-vnd.ProjectConceptorStressGen

SRCS= 	GenerateStressFixture.cpp \
		../../app/ProjectConceptorDefs.cpp \
		../../app/Config/ConfigManager.cpp \
		../../app/Config/ConfigWindow.cpp \
		../../app/Config/ConfigView.cpp \
		../../app/Config/ShortCutView.cpp \
		../../app/Config/KeyCaptureWindow.cpp \
		../../app/Config/PluginView.cpp \
		../../app/Config/PluginItem.cpp \
		../../app/Config/PluginInfoView.cpp \
		../../app/Config/MessageView.cpp \
		../../app/Commands/PCommand.cpp \
		../../app/Commands/PCommandManager.cpp \
		../../app/Document/PDocument.cpp \
		../../app/Document/PDocumentManager.cpp \
		../../app/Document/PDocLoader.cpp \
		../../app/Document/PEditor.cpp \
		../../app/Document/PEditorManager.cpp \
		../../app/Document/PWindow.cpp \
		../../app/Document/HelpManager.cpp \
		../../app/MainView.cpp \
		../../app/ToolBar/BaseItem.cpp \
		../../app/ToolBar/Mover.cpp \
		../../app/ToolBar/ToolBar.cpp \
		../../app/ToolBar/ToolBarSeperator.cpp \
		../../app/ToolBar/ToolItem.cpp \
		../../app/ToolBar/ToolMenu.cpp \
		../../app/ToolBar/ColorToolItem.cpp \
		../../app/ToolBar/FloatToolItem.cpp \
		../../app/ToolBar/PatternToolItem.cpp \
		../../app/Tools/AFont.cpp \
		../../app/Tools/PCSavePanel.cpp \
		../../app/Tools/Indexer.cpp \
		../../app/Tools/InputRequestView.cpp \
		../../app/Tools/InputRequest.cpp \
		../../app/Tools/MessageXmlReader.cpp \
		../../app/Tools/MessageXmlWriter.cpp \
		../../app/Tools/ShortCutFilter.cpp \
		../../app/Tools/TextViewCompleter.cpp \
		../../app/Tools/plugins/BasePlugin.cpp \
		../../app/Tools/plugins/PluginManager.cpp

RDEFS=

RSRCS=

LIBS= be root translation tracker localestub $(STDCPPLIBS) tinyxml

LIBPATHS=

SYSTEM_INCLUDE_PATHS =	/boot/system/develop/headers/private/support \
						/boot/system/develop/headers/private/shared \
						/boot/system/develop/headers/private/interface

LOCAL_INCLUDE_PATHS =	. \
						../../include \
						../../include/plugins \
						../../app \
						../../app/Commands \
						../../app/Config \
						../../app/Document \
						../../app/ToolBar \
						../../app/About \
						../../app/Tools

COMPILER_FLAGS =

LINKER_FLAGS =

TARGET_DIR	=  .

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
