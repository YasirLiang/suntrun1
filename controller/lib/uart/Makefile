SUBDIRS = $(shell ls -l |grep ^d |awk '{print $$9}')

ALL:$(SUBDIRS)

$(SUBDIRS):ECHO
	make -C $@

ECHO:
	@echo $(SUBDIRS)

.PHONY:clean
clean:
	make -C $(SUBDIRS) clean
