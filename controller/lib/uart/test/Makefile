all:uart_main test_matrix
uart_main:uart_main.c
	$(CC) -o $@ $^ $(CFGS) $(CCFLAGS) $(CFG_INC)
test_matrix:test_matrix.c
	$(CC) -o $@ $^ $(CFGS) $(CCFLAGS) $(CFG_INC)
clean:
	$(RM) uart_main test_matrix .*.sw?
