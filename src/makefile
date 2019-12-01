first: default

PLNAME = $(shell uname)

pkgupdate:
	@echo "---------PKG INFO GEN-----------"
	touch .PackageInfo
	rm .PackageInfo
	@echo "" >> .PackageInfo 
	@echo "#//do not edit this" >> .PackageInfo
	@echo "name			ProjectConceptor" >> .PackageInfo
#later use git describe --abbrev=0 --tags to use git tags
	@echo "version			0.0.6-$(GIT_REV)" >> .PackageInfo
	@echo "" >> .PackageInfo  
	@echo "architecture	$(shell getarch)" >> .PackageInfo
	@echo "summary			\"Projectconceptor a Graph editing suite\"" >> .PackageInfo  
	@echo "description		\"Projectconceptor is modular Graphediting suite with export and import support for freemind\"" >> .PackageInfo  
	@echo "packager		\"paradoxon <.will come later@later.de>\"" >> .PackageInfo  
	@echo "vendor			\"Paradoxon\"" >> .PackageInfo  
	@echo "licenses {" >> .PackageInfo  
	@echo "	\"MIT\"" >> .PackageInfo  
	@echo "}" >> .PackageInfo  
	@echo "copyrights {" >> .PackageInfo  
	@echo "	\"Copyright (C) 2005-2016 by Paradoxon\"" >> .PackageInfo  
	@echo "}" >> .PackageInfo  
	@echo "provides {" >> .PackageInfo  
	@echo "        ProjectConceptor = 0.0.6-$(GIT_REV)" >> .PackageInfo  
	@echo "}" >> .PackageInfo  
	@echo "requires {" >> .PackageInfo  
	@echo "	haiku >= r1~alpha4_pm-1" >> .PackageInfo  
	@echo "}" >> .PackageInfo  
	@echo "source-urls {" >> .PackageInfo
	@echo "	\"Download <https://github.com/Paradoxianer/ProjectConceptor/archive/master.zip>\"" >> .PackageInfo
	@echo "}" >> .PackageInfo
	@echo "" >> .PackageInfo

	@echo "UPDATED"
	@echo "--------------------------------"


INSTALL_DIR	= /boot/home/config/packages

#global setting global the app Dir
export APP_DIR		=	bin/apps/ProjectConceptor
export PLUGIN_DIR	=	Plugins

#global settings for setting the level of debug and optimization
#	specify the level of optimization that you desire
#	NONE, SOME, FULL
export OPTIMIZE= NONE
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

# 	Specify the codes for languages you are going to support in this
# 	application. The default "en" one must be provided too. "make catkeys"
# 	will recreate only the "locales/en.catkeys" file. Use it as a template
# 	for creating catkeys for other languages. All localization files must be
# 	placed in the "locales" subdirectory.
export LOCALES = en es de

app: pkgupdate app/makefile FORCE
	cd app && make

app_clean:
	cd app && make clean

app_install: app/makefile FORCE
	cd app && make install

app_catkeys: app/makefile FORCE
	cd app && make catkeys

app_bindcat: app/makefile FORCE
	cd app && make bindcatalogs


plugins: plugins/makefile FORCE
	cd plugins && make

plugins_clean:
	cd plugins && make clean

plugins_install: plugins/makefile FORCE
	cd plugins && make install

plugins_catkeys: plugins/makefile FORCE
	cd plugins && make catkeys
	
plugins_bindcat: plugins/makefile FORCE
	cd plugins && make bindcatalogs


translators: translators/makefile FORCE
	cd translators && make

translators_clean:
	cd translators && make clean

translators_install: translators/makefile FORCE
	cd translators && make install

translators_catkeys: translators/makefile FORCE
	cd translators && make catkeys
	
translators_bindcat: translators/makefile FORCE
	cd translators && make bindcatalogs


default: app plugins translators

clean: app_clean plugins_clean translators_clean

catkeys: app_catkeys plugins_catkeys translators_clean

bindcatalogs: app_bindcat plugins_bindcat translators_clean

install:: $(NAME)
	@echo "making a package"
	@cp .PackageInfo ../bin
	@package create -C ../bin ProjectConceptor-$(shell getarch)-$(if $(DEBUGGER),debug-$(GIT_REV)).hpkg
	@echo "installing package"
	@cp ProjectConceptor-$(shell getarch)-$(if $(DEBUGGER),debug-$(GIT_REV)).hpkg $(INSTALL_DIR)
	@echo "Installation done."


FORCE:
