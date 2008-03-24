first: default

app: app/makefile FORCE
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
install: app_install plugins_install

FORCE:
