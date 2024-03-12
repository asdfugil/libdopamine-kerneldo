all: libdopaminekerneldo.dylib

LDFLAGS += -shared -Wl,-rpath,/var/jb/usr/lib -Wl,-rpath,/var/jb/basebin -Wl,-rpath,/var/jb/basebin/fallback main.c ../../libjailbreak.tbd ../../libellekit.tbd

libdopaminekerneldo.dylib: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I../../include -o $@
	codesign -s - --force libdopaminekerneldo.dylib

clean:
	rm -f libdopaminekerneldo.dylib

.PHONY: all clean
