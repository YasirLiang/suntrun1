SRC_FILES += $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRC_FILES))

%.o:%.c
	$(CC) $(CFG_INC) $(CFLAGS) -c -o $@ $<

avdecc_ctl:$(OBJS)
	$(CC) -o avdecc_ctl $(OBJS) $(CFLAGS) 
	mv avdecc_ctl ../

.PHONY:clean
clean:
	rm -rf $(OBJS) 
