## BeOS Generic Makefile v2.0 ##

## Real shared library every plugin/translator links against for the small
## set of app-side symbols they actually need (ProjectConceptorDefs.cpp's
## P_C_* field-name constants, PCommand's base implementation) - replaces
## ProjectConceptor.a, which was never a real archive, just a symlink to
## the app's own executable (issue #110). That meant a translator loaded
## without ProjectConceptor running (Tracker previewing a .pcd, any other
## Translation-Kit-using app) failed to resolve these same symbols at
## relocation time - confirmed live via `translate` + syslog before this
## fix existed. Scope (just these two files) verified empirically: compiled
## them standalone and test-linked a plain command plugin, the biggest
## editor plugin, and a translator against them in place of the old .a -
## all resolved cleanly, nothing else needed.

NAME= libProjectConceptor.so

TYPE= SHARED

SRCS=	../ProjectConceptorDefs.cpp \
		../Commands/PCommand.cpp

RDEFS=

RSRCS=

LIBS= be localestub

LIBPATHS=

SYSTEM_INCLUDE_PATHS =	/boot/system/develop/headers/private/support \
						/boot/system/develop/headers/private/shared \
						/boot/system/develop/headers/private/interface

LOCAL_INCLUDE_PATHS =	. \
						.. \
						../Commands \
						../../include \
						../../include/plugins

COMPILER_FLAGS =

LINKER_FLAGS =

TARGET_DIR	=  .

INSTALL_DIR	= /boot/apps/ProjectConceptor

## include the makefile-engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

default .DEFAULT:
	mkdir -p ../../../$(APP_DIR)
	cp -rf $(NAME) ../../../$(APP_DIR)

install:: default
	mkdir -p $(INSTALL_DIR)
	cp -rf $(TARGET) $(INSTALL_DIR)
