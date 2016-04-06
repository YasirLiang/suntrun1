CC = arm-linux-gcc
AR = arm-linux-ar
LD = arm-linux-ld
OBJDUMP = arm-linux-objdump
export CC AR LD

ROOTPATH = $(shell pwd)
CONTROLINCPATH = $(ROOTPATH)/controller/lib/include
CFGS = -Wall -g
CCFLAGS = -lavdecc-host -ljdksavdecc -lpthread -lrt -lreadline -lncurses -lsqlite3 -L$(ROOTPATH)/controller/lib -L$(ROOTPATH)/lib
CFG_INC += -I$(ROOTPATH)/controller/app/include \
		   -I$(ROOTPATH)/lib/include/jdksavdecc \
		   -I$(CONTROLINCPATH)/avdecc \
		   -I$(CONTROLINCPATH)/conference_host_with_endstation\
		   -I$(CONTROLINCPATH)/conference_network\
		   -I$(CONTROLINCPATH)/consultative_encapsulation \
		   -I$(CONTROLINCPATH)/packet_events \
		   -I$(CONTROLINCPATH)/host_controller_debug \
		   -I$(CONTROLINCPATH)/native_proccess \
		   -I$(CONTROLINCPATH)/system_work_tool_uits \
		   -I$(CONTROLINCPATH)/upper_computer \
		   -I$(CONTROLINCPATH)/end_station_manager \
		   -I$(CONTROLINCPATH)/message_func \
		   -I$(CONTROLINCPATH)/common\
		   -I$(CONTROLINCPATH)/connect_manager\
		   -I$(CONTROLINCPATH)/send_module\
		   -I$(CONTROLINCPATH)/camera_module\
		   -I$(CONTROLINCPATH)/uart\
		   -I$(CONTROLINCPATH)/timer_pthread\
		   -I$(CONTROLINCPATH)/control_matrix\
		   -I$(CONTROLINCPATH)/system_database\
		   -I$(CONTROLINCPATH)/ui_encap\
		   -I$(CONTROLINCPATH)/host_connect\
		   -I$(CONTROLINCPATH)/menu_displays
export CFG_INC CFGS CCFLAGS ROOTPATH

SUBDIRS=$(shell ls -l | grep ^d | awk '{if($$9 != "controller") print $$9}')
APP_SUB=$(shell ls -l | grep ^d | awk '{if($$9 == "controller") print $$9}')

ALL:$(SUBDIRS) APP_SUB

$(SUBDIRS):ECHO
	make -C $@

APP_SUB:ECHO
	make -C controller
#@$(OBJDUMP) -alD avdecc_ctl > avdecc_ctl.txt
#@$(OBJDUMP) -S -d avdecc_ctl > avdecc_ctl.txt
	@$(OBJDUMP) -d avdecc_ctl > avdecc_ctlDump

ECHO:
	@echo $(SUBDIRS)

.PHONY:cleanall clean
clean:
	make -C controller clean
	@$(RM) avdecc_ctl system.dat address.dat avdecc_ctl.txt avdecc_ctlDump preset_plist.dat
cleanall:
	make -C lib clean
	make -C controller clean
	@$(RM) avdecc_ctl system.dat address.dat avdecc_ctl.txt avdecc_ctlDump preset_plist.dat
