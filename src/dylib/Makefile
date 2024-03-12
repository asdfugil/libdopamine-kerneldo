all: libdopaminekerneldo.dylib

libdopaminekerneldo.dylib: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-rpath,/var/jb/basebin main.c ../../libjailbreak.tbd -o $@
	codesign -s - --force libdopaminekerneldo.dylib

clean:
	rm -f libdopaminekerneldo.dylib

.PHONY: all clean
