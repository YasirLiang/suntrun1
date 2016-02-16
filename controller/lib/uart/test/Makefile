all:uart_main test_matrix
uart_main:uart_main.c
	@echo "\033[32m$(CC) -o $@ $^\033[0m"
	@$(CC) -o $@ $^ $(CFGS) $(CCFLAGS) $(CFG_INC)
test_matrix:test_matrix.c
	@echo "\033[32m$(CC) -o $@ $^\033[0m"
	@$(CC) -o $@ $^ $(CFGS) $(CCFLAGS) $(CFG_INC)
clean:
	$(RM) uart_main test_matrix .*.sw?
