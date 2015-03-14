first: default

PLNAME = $(shell uname)

ifeq ($(PLNAME), Linux)
	COMMAND = "\#define SVN_REV_STR \"`svn info http://svn.berlios.de/svnroot/repos/projectconcepto/trunk | grep Revision | awk '{print $$2}'`\""
else
	COMMAND = "\#define SVN_REV_STR \"`svn info | grep Revision | awk '{print $$2}'`\""
endif

svnupdate:
	@echo "---------SVN INFO GEN-----------"
	touch app/About/AboutSvnInfo.h
	rm app/About/AboutSvnInfo.h
	@echo "" >> app/About/AboutSvnInfo.h 
	@echo "#ifndef SVN_INFO_H" >> app/About/AboutSvnInfo.h 
	@echo "#define SVN_INFO_H" >> app/About/AboutSvnInfo.h 
	@echo "" >> app/About/AboutSvnInfo.h  
	@echo "//do not edit this" >> app/About/AboutSvnInfo.h
	@echo $(COMMAND) >> app/About/AboutSvnInfo.h
	@echo "" >> app/About/AboutSvnInfo.h 
	@echo "#endif" >> app/About/AboutSvnInfo.h 
	@echo "UPDATED"
	@echo "--------------------------------"


INSTALL_DIR	= /boot/home/config/packages

#global setting global the app Dir
export APP_DIR		=	bin/apps/ProjectConceptor
export PLUGIN_DIR	=	Plugins

#global settings for setting the level of debug and optimization
#	specify the level of optimization that you desire
#	NONE, SOME, FULL
export OPTIMIZE= FULL
#	specify any preprocessor symbols to be defined.  The symbols
#	will be set to a value of 1.  For example specify DEBUG if you want
#	DEBUG=1 to be set when compiling.
export DEFINES= DEBUG
#	specify special warning levels
#	if unspecified default warnings will be used
#	NONE = supress all warnings
#	ALL = enable all warnings
export WARNINGS = ALL
#	specify whether image symbols will be created
#	so that stack crawls in the debugger are meaningful
#	if TRUE symbols will be created
export SYMBOLS = TRUE
#	specify debug settings
#	if TRUE will allow application to be run from
#	a source-level debugger
export DEBUGGER = TRUE

app: svnupdate app/makefile FORCE
	cd app && make

plugins: plugins/makefile FORCE
	cd plugins && make

app_clean:
	cd app && make clean

plugins_clean:
	cd plugins && make clean

app_install: app/makefile FORCE
	cd app && make install



plugins_install: plugins/makefile FORCE
	cd plugins && make install

default: app plugins
clean: app_clean plugins_clean
#install: app_install plugins_install

install:: $(NAME)
	@echo "making a package"
	@cp .PackageInfo ../bin
	@package create -C ../bin ProjectConceptor_gcc.hpkg
	@echo "installing package"
	@cp ProjectConceptor_gcc.hpkg $(INSTALL_DIR)
	@echo "Installation done."



FORCE:
