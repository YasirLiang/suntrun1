CC = gcc
AR = ar
LD = ld
OBJDUMP = objdump
export CC AR LD

ROOTPATH = $(shell pwd)
#CFLAGS = -Wall -O -O2 -lavdecc-host -ljdksavdecc -lpthread -lreadline -lrt -L$(ROOTPATH)/controller/lib -L$(ROOTPATH)/lib
CFLAGS = -Wall -lavdecc-host -ljdksavdecc -lpthread -lreadline -lrt -L$(ROOTPATH)/controller/lib -L$(ROOTPATH)/lib
#CFLAGS = -Wall -O -O2 -g -lavdecc-host -ljdksavdecc -lpthread -lreadline -lrt -L$(ROOTPATH)/controller/lib -L$(ROOTPATH)/lib
CFG_INC += -I$(ROOTPATH)/controller/include \
		   -I$(ROOTPATH)/controller/app/include \
		   -I$(ROOTPATH)/lib/include/jdksavdecc \
		   -I$(ROOTPATH)/controller/lib/include/avdecc \
		   -I$(ROOTPATH)/controller/lib/include/conference_host_with_endstation\
		   -I$(ROOTPATH)/controller/lib/include/conference_network\
		   -I$(ROOTPATH)/controller/lib/include/consultative_encapsulation \
		   -I$(ROOTPATH)/controller/lib/include/packet_events \
		   -I$(ROOTPATH)/controller/lib/include/host_controller_debug \
		   -I$(ROOTPATH)/controller/lib/include/native_proccess \
		   -I$(ROOTPATH)/controller/lib/include/system_work_tool_uits \
		   -I$(ROOTPATH)/controller/lib/include/upper_computer \
		   -I$(ROOTPATH)/controller/lib/include/end_station_manager \
		   -I$(ROOTPATH)/controller/lib/include/message_func \
		   -I$(ROOTPATH)/controller/lib/include/common\
		   -I$(ROOTPATH)/controller/lib/include/connect_manager\
		   -I$(ROOTPATH)/controller/lib/include/send_module\
		   -I$(ROOTPATH)/controller/lib/include/camera_module\
		   -I$(ROOTPATH)/controller/lib/include/uart


export CFG_INC CFLAGS ROOTPATH

SUBDIRS=$(shell ls -l | grep ^d | awk '{if($$9 != "controller") print $$9}')
APP_SUB=$(shell ls -l | grep ^d | awk '{if($$9 == "controller") print $$9}')

ALL:$(SUBDIRS) APP_SUB

$(SUBDIRS):ECHO
	make -C $@

APP_SUB:ECHO
	make -C controller
	@$(OBJDUMP) -alD avdecc_ctl > avdecc_ctl.txt
	@$(OBJDUMP) -d avdecc_ctl > avdecc_ctlDump

ECHO:
	@echo $(SUBDIRS)

.PHONY:cleanall clean
clean:
	make -C controller clean
	@$(RM) avdecc_ctl system.dat address.dat avdecc_ctl.txt avdecc_ctlDump
cleanall:
	make -C lib clean
	make -C controller clean
	@$(RM) avdecc_ctl system.dat address.dat avdecc_ctl.txt avdecc_ctlDump
