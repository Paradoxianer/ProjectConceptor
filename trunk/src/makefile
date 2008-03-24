first: default

app: app/makefile FORCE
	cd app && make

plugins: plugins/makefile FORCE
	cd plugins && make

app_clean:
	cd app && make clean

plugins_clean:
	cd plugins && make clean

default: app plugins
clean: app_clean plugins_clean

FORCE:
