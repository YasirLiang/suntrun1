SRC_FILES += $(wildcard jdksavdecc/*.c) 

OBJS := $(patsubst %.c, %.o, $(SRC_FILES)) 

DIR=$(notdir $(OBJS))

LIB=libjdksavdecc.a

all:$(OBJS) LIB

$(OBJS):%.o:%.c
	@echo "\033[32m$(CC) -o $@ $< \033[0m"
	@$(CC) $(CFLAGS) $(CFG_INC) -c -o $@ $<

LIB:$(OBJS)
ifeq ($(LIB), $(wildcard $(LIB)))
else
	@echo $(AR) rc $(LIB) $(DIR)
endif

	@$(AR) rc $(LIB) $(OBJS)

.PHONY:clean
clean:
	@echo $(RM) $(DIR) $(LIB)
	@$(RM) $(OBJS) $(LIB)
