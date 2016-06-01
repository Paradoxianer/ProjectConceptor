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

define default-target
  def:: ; cd $1 && make 
endef

define install-target
  inst:: ; cd $1 && make install
endef

define clean-target
  cln:: ; cd $1 && make clean
endef

define catkeys-target
  cat:: ; cd $1 && make catkeys 
endef

define bindcatalogs-target
  bind:: ; cd $1 && make bindcatalogs
endef

default: \
	$(foreach dir,$(plugins),$(eval $(call default-target,$(dir))))

install: \
	$(foreach dir,$(plugins),$(eval $(call install-target,$(dir))))

clean: \
	$(foreach dir,$(plugins),$(eval $(call clean-target,$(dir))))

catkeys: \
	$(foreach dir,$(plugins),$(eval $(call catkeys-target,$(dir))))

bindcatalogs: \
	$(foreach dir,$(plugins),$(eval $(call bindcatalogs-target,$(dir))))

FORCE:
