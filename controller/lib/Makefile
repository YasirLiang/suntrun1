SUBDIRS=$(shell ls -l |grep ^d | awk '{if($$9 == "uart") print $$9}')
SRC_FILE = $(wildcard avdecc/*.c) $(wildcard camera_module/*.c)\
		    $(wildcard common/*.c) $(wildcard upper_computer/*.c)\
		    $(wildcard conference_host_with_endstation/*.c) $(wildcard conference_network/*.c)\
		    $(wildcard connect_manager/*.c) $(wildcard end_station_manager/*.c)\
		    $(wildcard message_func/*.c) $(wildcard native_proccess/*.c)\
		    $(wildcard packet_events/*.c) $(wildcard send_module/*.c) \
		    $(wildcard system_work_tool_uits/*.c) $(wildcard uart/*.c)\
		    $(wildcard timer_pthread/*.c) $(wildcard control_matrix/*.c)
DEPS = $(patsubst %.c, %.d, $(SRC_FILE))
OBJS = $(patsubst %.c, %.o, $(SRC_FILE))
DIR = $(notdir $(OBJS))

LIB=libavdecc-host.a
all:LIB $(SUBDIRS)

LIB:$(OBJS)
ifeq ($(LIB), $(wildcard $(LIB)))
else
	@echo $(AR) rc $(LIB) $(DIR)
endif
	@$(AR) rc $(LIB) $(OBJS)

$(OBJS):%.o:%.c
	@echo "\033[32m$(CC) -o $@ $< \033[0m"
	@$(CC) -MM $(CFG_INC) -MT"$*.d" -MT"$(<:.c=.o)" $< > $*.d
	@$(CC) -c $(CFG_INC) $(CFGS) $< -o $@
	
sinclude $(DEPS)

$(SUBDIRS):ECHO
	make -C $@

ECHO:
	@echo $(SUBDIRS)

.PHONY:clean
clean:
	@echo $(RM) $(DIR) $(LIB) .*.sw?
	@$(RM) $(OBJS) $(LIB) $(DEPS) .*.sw?
	make -C $(SUBDIRS) clean
