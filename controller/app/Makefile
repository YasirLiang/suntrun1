CUR_SOURCE = $(wildcard ./*.c)
CUR_OBJS = $(patsubst %.c, %.o, $(CUR_SOURCE))

BIN=avdecc_ctl

all:$(CUR_OBJS) BIN

$(CUR_OBJS):%.o:%.c
	@echo "$(CC) $< -o $@"
	@$(CC) $(CFLAGS) $(CFG_INC) -c  -o $@ $<

BIN:$(CUR_OBJS)
	@echo "$(CC) -o $(BIN)"
	@$(CC) -o $(BIN) $(CUR_OBJS) $(CFLAGS)
	@cp $(BIN) $(ROOTPATH) 

.PHONY:clean
clean:
	@echo $(RM) $(CUR_OBJS) $(BIN) .*.sw?
	@$(RM) $(CUR_OBJS) $(BIN) .*.sw?
