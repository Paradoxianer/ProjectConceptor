plugins = \
	AddAttributeCommand \
	AskCommand \
	BatchCommand \
	ChangeValueCommand \
	CopyCommand \
	DeleteCommand \
	FindCommand \
	ForEachCommand \
	GraphEditor \
	GroupCommand \
	IfCommand \
	InsertCommand \
	LayoutEditor \
	LayoutCommand \
	MacroEditor \
	MoveCommand \
	NavigatorEditor \
	PasteCommand \
	RememberCommand \
	RemoveAttributeCommand \
	RepeatCommand \
	ResizeCommand \
	SelectCommand \
	SleepCommand

# LayoutEditor must build before LayoutCommand: LayoutCommand links against
# LayoutEditor's already-built add-on binary (DotLayouter/
# LayoutCommandBuilder live there) instead of recompiling those sources a
# second time - sharing a .cpp via a relative SRCS path across two plugin
# directories that both produce the same relative objects.../<name>.o path
# hits a VPATH collision in the makefile-engine (make resolves the target
# against whichever sibling directory already has that object, then the
# link step can't find a local copy) - not alphabetical order otherwise.

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
	$(MAKE) -C GraphEditor bindcatalogs; \
	$(MAKE) -C NavigatorEditor bindcatalogs; \



FORCE:
