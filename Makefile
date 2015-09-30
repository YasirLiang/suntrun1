CC = gcc
AR = ar
LD = ld
export CC AR LD

ROOTPATH = $(shell pwd)
CFLAGS = -Wall -lavdecc-host -lpthread -lreadline -lrt -L$(ROOTPATH)
CFG_INC += -I$(ROOTPATH)/controller/include \
		   -I$(ROOTPATH)/lib/include/avdecc \
		   -I$(ROOTPATH)/lib/include/jdksavdecc \
		   -I$(ROOTPATH)/lib/include/conference_host_with_endstation\
		   -I$(ROOTPATH)/lib/include/conference_network\
		   -I$(ROOTPATH)/lib/include/consultative_encapsulation \
		   -I$(ROOTPATH)/lib/include/packet_events \
		   -I$(ROOTPATH)/lib/include/host_controller_debug \
		   -I$(ROOTPATH)/lib/include/native_proccess \
		   -I$(ROOTPATH)/lib/include/system_work_tool_uits \
		   -I$(ROOTPATH)/lib/include/upper_computer \
		   -I$(ROOTPATH)/lib/include/end_station_manager

export CFG_INC CFLAGS ROOTPATH

ALL:
	make -C lib
	make -C controller

.PHONY:clean
clean:
	-rm -rf libavdecc-host.a avdecc_ctl
	make -C lib clean
	make -C controller clean
