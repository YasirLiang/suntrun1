CC = gcc
AR = ar
LD = ld
OBJDUMP = objdump
export CC AR LD

ROOTPATH = $(shell pwd)
CFLAGS = -Wall -O -O2 -lavdecc-host -lpthread -lreadline -lrt -L$(ROOTPATH) -I$(ROOTPATH)/lib 
#CFLAGS = -Wall -lavdecc-host -lpthread -lreadline -lrt -L$(ROOTPATH) -I$(ROOTPATH)/lib
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
		   -I$(ROOTPATH)/lib/include/end_station_manager \
		   -I$(ROOTPATH)/lib/include/message_func \
		   -I$(ROOTPATH)/lib/include/common\
		   -I$(ROOTPATH)/lib/include/connect_manager\
		   -I$(ROOTPATH)/lib/include/send_module


export CFG_INC CFLAGS ROOTPATH

ALL:
	make -C lib
	make -C controller
#	$(OBJDUMP) -alD avdecc_ctl > avdecc_ctl.txt

.PHONY:clean
clean:
	-rm -rf libavdecc-host.a avdecc_ctl avdecc_ctl.txt address.dat system.dat
	make -C lib clean
	make -C controller clean
