first: default

# make install: a package of this tree, built like a release
# (tools/make-release.sh) and put in place of the installed one. The version
# is the last tag and the commits since, e.g. 0.5.1.16, which comes after
# that release; without git (the VM's copy) dev.sh passes PACKAGE_VERSION.
PACKAGE_VERSION	?= $(shell git describe --tags --match 'v[0-9]*' 2>/dev/null \
	| sed -e 's/^v//' -e 's/-\([0-9]*\)-g.*/.\1/')
PACKAGES	= /boot/home/config/packages

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

lib: app/lib/makefile FORCE
	cd app/lib && make

lib_clean:
	cd app/lib && make clean

app: lib app/makefile FORCE
	cd app && make

app_clean:
	cd app && make clean

app_catkeys: app/makefile FORCE
	cd app && make catkeys

app_bindcat: app/makefile FORCE
	cd app && make bindcatalogs


plugins: lib plugins/makefile FORCE
	cd plugins && make

plugins_clean:
	cd plugins && make clean

plugins_catkeys: plugins/makefile FORCE
	cd plugins && make catkeys
	
plugins_bindcat: plugins/makefile FORCE
	cd plugins && make bindcatalogs


translators: lib translators/makefile FORCE
	cd translators && make

translators_clean:
	cd translators && make clean

translators_catkeys: translators/makefile FORCE
	cd translators && make catkeys
	
translators_bindcat: translators/makefile FORCE
	cd translators && make bindcatalogs


default: app plugins translators

clean: lib_clean app_clean plugins_clean translators_clean

catkeys: app_catkeys plugins_catkeys translators_catkeys

bindcatalogs: app_bindcat plugins_bindcat translators_bindcat

install: FORCE
	@if [ -z "$(PACKAGE_VERSION)" ]; then \
		echo "make install: git describe found no version tag, call" \
			"\"make install PACKAGE_VERSION=<version>\"" >&2; exit 1; fi
	cd .. && tools/make-release.sh $(PACKAGE_VERSION)
	@# the old package and its debug info out, they require each other
	@set -e; arch=$$(getarch); primary=$$(getarch -p); \
	name=projectconceptor; \
	if [ "$$arch" != "$$primary" ]; then name=projectconceptor_$$arch; fi; \
	rm -f $(PACKAGES)/$$name-[0-9]*.hpkg $(PACKAGES)/$${name}_debuginfo-[0-9]*.hpkg; \
	cp ../release/$$name-$(PACKAGE_VERSION)-1-$$primary.hpkg \
		../release/$${name}_debuginfo-$(PACKAGE_VERSION)-1-$$primary.hpkg $(PACKAGES)/; \
	echo "Installed $$name $(PACKAGE_VERSION)."


FORCE:
