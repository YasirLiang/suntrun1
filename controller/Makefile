SUBDIRS=$(shell ls -l | grep ^d | awk '{if($$9 != "app") print $$9}')
APPDIR=$(shell ls -l | grep ^d | awk '{if($$9 == "app") print $$9}')

ALL:$(SUBDIRS) APPDIR

$(SUBDIRS):ECHO
	make -C $@

APPDIR:ECHO
	make -C app

ECHO:
	@echo $(SUBDIRS)

.PHONY:clean
clean:
	make -C app $@
	make -C lib $@
