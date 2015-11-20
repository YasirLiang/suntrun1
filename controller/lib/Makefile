SRC_FILE += $(wildcard avdecc/*.c) $(wildcard camera_module/*.c)\
		    $(wildcard common/*.c) $(wildcard upper_computer/*.c)\
		    $(wildcard conference_host_with_endstation/*.c) $(wildcard conference_network/*.c)\
		    $(wildcard connect_manager/*.c) $(wildcard end_station_manager/*.c)\
		    $(wildcard message_func/*.c) $(wildcard native_proccess/*.c)\
		    $(wildcard packet_events/*.c) $(wildcard send_module/*.c) \
		    $(wildcard system_work_tool_uits/*.c) $(wildcard uart/*.c)
		    
OBJS = $(patsubst %.c, %.o, $(SRC_FILE))

DIR = $(notdir $(OBJS))

LIB=libavdecc-host.a

ALL:$(OBJS) LIB
$(OBJS):%o:%c
	@echo $< -o $@
	@$(CC) -c -o $@ $< $(CFG_INC) $(CFLAGS) 

LIB:$(OBJS)
	@echo $(AR) rc $(LIB) $(DIR)
	@$(AR) rc $(LIB) $(OBJS)

.PHONY:clean
clean:
	@echo $(RM) $(DIR) $(LIB) .*.sw?
	@$(RM) $(OBJS) $(LIB) .*.sw?
