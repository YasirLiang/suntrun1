SRC_FILES = $(wildcard jdksavdecc/*.c) 

OBJS = $(patsubst %.c, %.o, $(SRC_FILES)) 
DEPS = $(patsubst %.c, %.d, $(SRC_FILES)) 
DIR=$(notdir $(OBJS))

LIB=libjdksavdecc.a

all:LIB

%.o:%.c
	$(CC) -MM $(CFG_INC) -MT"$*.d" -MT"$(<:.c=.o)" $< > $*.d
	$(CC) $(CFGS) $(CFG_INC) -c -o $@ $<

LIB:$(OBJS)
ifeq ($(LIB), $(wildcard $(LIB)))
else
	@echo $(AR) rc $(LIB) $(DIR)
endif
	@$(AR) rc $(LIB) $(OBJS)

sinclude $(DEPS)

.PHONY:clean
clean:
	@echo $(RM) $(DIR) $(LIB)
	@$(RM) $(OBJS) $(LIB) $(DEPS)
