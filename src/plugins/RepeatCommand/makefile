## BeOS Generic Makefile v2.0 ##

NAME= Repeat

TYPE= SHARED

APP_MIME_SIG = application/x-vnd.ProjectConceptor-plugin

SRCS= 	Repeat.cpp \
		RepeatPlugin.cpp

RSRCS=

LIBS= be ../../../$(APP_DIR)/libProjectConceptor.so

LIBPATHS=

SYSTEM_INCLUDE_PATHS =

LOCAL_INCLUDE_PATHS = \
		../../include \
		../../include/plugins \
		../../app/Commands

COMPILER_FLAGS =

LINKER_FLAGS = -Xlinker -rpath=\$$ORIGIN/../..


## include the makefile-engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

default .DEFAULT:
	mkdir -p ../../../$(APP_DIR)/$(PLUGIN_DIR)/Commands
	cp -rf  $(OBJ_DIR)/$(NAME) ../../../$(APP_DIR)/$(PLUGIN_DIR)/Commands
