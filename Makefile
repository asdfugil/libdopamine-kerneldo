CC != xcrun --find clang
SYSROOT != xcrun -sdk iphoneos --show-sdk-path
CFLAGS = -arch arm64 -arch arm64e -Os -isysroot $(SYSROOT) -Os -Wall -Wextra -miphoneos-version-min=15.0

export CC SYSROOT CFLAGS

all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

.PHONY: all clean
