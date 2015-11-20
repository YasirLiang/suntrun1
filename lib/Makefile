SRC_FILES += $(wildcard jdksavdecc/*.c) 

OBJS := $(patsubst %.c, %.o, $(SRC_FILES)) 

DIR=$(notdir $(OBJS))

LIB=libjdksavdecc.a

all:$(OBJS) LIB

$(OBJS):%.o:%.c
	@echo "$(CC) $< -o $@"
	@$(CC) $(CFLAGS) $(CFG_INC) -c $< -o $@

LIB:$(OBJS)
	@echo "$(AR) rc $(LIB) $(DIR)"
	@$(AR) rc $(LIB) $(OBJS)

.PHONY:clean
clean:
	@echo $(RM) $(DIR) $(LIB)
	@$(RM) $(OBJS) $(LIB)
