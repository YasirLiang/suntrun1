SRC_FILES += $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRC_FILES))

%.o:%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CFG_INC)

avdecc_ctl:$(OBJS) 
	$(CC) -o avdecc_ctl $(OBJS) $(CFLAGS) 
	mv avdecc_ctl ../

.PHONY:clean
clean:
	rm -rf $(OBJS) avdecc_ctl
