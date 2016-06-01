plugins = \
	AddAttributeCommand \
	ChangeValueCommand \
	CopyCommand \
	DeleteCommand \
	FindCommand \
	FreeMindTranslator \
	GraphEditor \
	GroupCommand \
	InsertCommand \
	MoveCommand \
	NavigatorEditor \
	PasteCommand \
	RemoveAttributeCommand \
	ResizeCommand \
	SelectCommand \
	StandardTranslator


default:
	for dir in $(plugins); do \
		$(MAKE) -C $$dir; \
	done

clean: 
	for dir in $(plugins); do \
		$(MAKE) -C $$dir clean; \
	done    

install: 
	for dir in $(plugins); do \
		$(MAKE) -C $$dir install; \
	done


catkeys:
	for dir in $(plugins); do \
		$(MAKE) -C $$dir catkeys; \
	done

bindcatalogs:
	for dir in $(plugins); do \
		$(MAKE) -C $$dir bindcatalogs; \
	done

    
 
FORCE:
