translators = \
	FreeMind \
	Standard 


default:
	for dir in $(translators); do \
		$(MAKE) -C $$dir; \
	done

clean: 
	for dir in $(translators); do \
		$(MAKE) -C $$dir clean; \
	done    

install: 
	for dir in $(translators); do \
		$(MAKE) -C $$dir install; \
	done


catkeys:
	for dir in $(translators); do \
		$(MAKE) -C $$dir catkeys; \
	done

bindcatalogs:
	for dir in $(translators); do \
		$(MAKE) -C $$dir bindcatalogs; \
	done

    
 
FORCE:
