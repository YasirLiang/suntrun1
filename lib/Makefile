SRC_FILES += $(wildcard jdksavdecc/*.c) 

DIR=$(notdir $(SRC_FILES))

OBJS := $(patsubst %.c, %.o, $(SRC_FILES)) 

%.o:%.c
	@echo "$(CC) $< -o $@"
	@$(CC) $(CFLAGS) $(CFG_INC) -c $< -o $@

libjdksavdecc.a:$(OBJS)
	@echo "$(AR) rc libjdksavdecc.a $(DIR)"
	@$(AR) rc libjdksavdecc.a $(OBJS)

.PHONY:clean
clean:
	rm -rf $(OBJS) libjdksavdecc.a
