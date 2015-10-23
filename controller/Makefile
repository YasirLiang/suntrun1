SRC_FILES += $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRC_FILES))

%.o:%.c
	$(CC) $(CFLAGS) $(CFG_INC) -c -o $@ $<

avdecc_ctl:$(OBJS)
	$(CC) -o avdecc_ctl $(OBJS) $(CFLAGS) 
	cp avdecc_ctl ../

.PHONY:clean
clean:
	rm -rf $(OBJS) avdecc_ctl
