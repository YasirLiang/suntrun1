SRC_FILES += $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRC_FILES))

%.o:%.c
	@echo "$(CC) $< -o $@"
	@$(CC) $(CFLAGS) $(CFG_INC) -c $< -o $@

avdecc_ctl:$(OBJS) 
	$(CC) $(OBJS) $(CFLAGS) -o avdecc_ctl 
	cp avdecc_ctl $(ROOTPATH)

.PHONY:clean
clean:
	rm -rf $(OBJS) avdecc_ctl
