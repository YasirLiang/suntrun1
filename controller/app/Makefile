CUR_SOURCE = $(wildcard ./*.c)
CUR_OBJS = $(patsubst %.c, %.o, $(CUR_SOURCE))
DEPS = $(patsubst %.c, %.d, $(CUR_SOURCE))

BIN=avdecc_ctl

all:$(CUR_OBJS) BIN

$(CUR_OBJS):%.o:%.c
	@echo "\033[32m$(CC) -o $@ $< \033[0m"
	@$(CC) $(CFLAGS) $(CFG_INC) -c  -o $@ $<

BIN:$(CUR_OBJS)
ifeq ($(BIN), $(wildcard $(BIN)))
else
	@echo $(CC) -o $(BIN) $(CUR_OBJS) $(CFLAGS)
endif
	@$(CC) -o $(BIN) $(CUR_OBJS) $(CFLAGS) $(CFG_INC)
	@cp $(BIN) $(ROOTPATH) 

sinclude $(CUR_OBJS:.o=.d)
%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFG_INC) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY:clean
clean:
	@echo $(RM) $(CUR_OBJS) $(BIN) $(DUMPTXT) .*.sw?
	@$(RM) $(CUR_OBJS) $(BIN) $(DUMPTXT) $(CUR_OBJS) $(DEPS) .*.sw?
