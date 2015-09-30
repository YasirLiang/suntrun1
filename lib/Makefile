SRC_FILE += $(wildcard src/avdecc/*.c) $(wildcard ls src/jdksavdecc/*.c)\
			$(wildcard src/conference_host_with_endstation/*.c) $(wildcard src/conference_network/*.c)\
			$(wildcard src/consultative_encapsulation/*.c) $(wildcard src/packet_events/*.c)\
			$(wildcard src/native_proccess/*.c) $(wildcard src/system_work_tool_uits/*.c)\
			$(wildcard src/upper_computer/*.c) $(wildcard src/end_station_manager/*.c)

OBJECTS = $(patsubst %.c, %.o, $(SRC_FILE))

%.o:%.c
	$(CC) $(CFG_INC) $(CFLAGS) -c -o $@ $<

libavdecc-host.a:$(OBJECTS)
	$(AR) rc libavdecc-host.a $(OBJECTS)
	chmod 777 libavdecc-host.a
	mv libavdecc-host.a ../

.PHONY:clean
clean:
	rm -rf $(OBJECTS)
	
