SRC_FILE += $(wildcard *.c) 

OBJECTS = $(patsubst %.c, %.o, $(SRC_FILE))

%.o:%.c
	@echo $(CC) -o $@ $<
	@$(CC) $(CFLAGS) $(CFG_INC) -c -o $@ $<

.PHONY:clean
clean:
	rm -rf $(OBJECTS) .*.sw?
	
