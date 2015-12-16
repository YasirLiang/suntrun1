LIB=libavdecc-host.a
all:LIB

SUBDIRS=$(shell ls -l |grep ^d | awk '{if($$9 == "uart") print $$9}')
SRC_FILE += $(wildcard avdecc/*.c) $(wildcard camera_module/*.c)\
		    $(wildcard common/*.c) $(wildcard upper_computer/*.c)\
		    $(wildcard conference_host_with_endstation/*.c) $(wildcard conference_network/*.c)\
		    $(wildcard connect_manager/*.c) $(wildcard end_station_manager/*.c)\
		    $(wildcard message_func/*.c) $(wildcard native_proccess/*.c)\
		    $(wildcard packet_events/*.c) $(wildcard send_module/*.c) \
		    $(wildcard system_work_tool_uits/*.c) $(wildcard uart/*.c)
DEPS = $(patsubst %.c, %.d, $(SRC_FILE))
OBJS = $(patsubst %.c, %.o, $(SRC_FILE))
DIR = $(notdir $(OBJS))

LIB:$(OBJS)
	@$(AR) rc $(LIB) $(OBJS)

$(OBJS):%o:%c
	@echo "\033[32m$(CC) -o $@ $< \033[0m"
	$(CC) -c $(CFG_INC) $(CFLAGS) $< -o $@
	
-include $(OBJS:.o=.d)
%.d:%.c
	set -e; rm -f $@; \
	$(CC) -MM $(CFG_INC) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(SUBDIRS):ECHO
	make -C $@

ECHO:
	@echo $(SUBDIRS)

.PHONY:clean
clean:
	@echo $(RM) $(DIR) $(LIB) .*.sw?
	@$(RM) $(OBJS) $(LIB) $(DEPS) .*.sw?
	make -C $(SUBDIRS) clean
