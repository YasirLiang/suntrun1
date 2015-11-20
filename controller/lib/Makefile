lib_all:
	make -C src

libavdecc-host.a:$(OBJECTS)
	$(AR) rc libavdecc-host.a $(OBJECTS)
	chmod 777 libavdecc-host.a
	cp libavdecc-host.a ../

.PHONY:clean
clean:
	rm -rf $(OBJECTS) libavdecc-host.a
	
