# libdopamine-kerneldo

Use kernel ucred on Dopamine. Insert with `DYLD_INSERT_LIBRARIES`

## Usage

### mount_tmpfs
```
DYLD_INSERT_LIBRARIES=/var/jb/usr/lib/libdopaminekerneldo.dylib mount_tmpfs /cores

```

### unmount
```
DYLD_INSERT_LIBRARIES=/var/jb/usr/lib/libdopaminekerneldo.dylib umount /cores

```

### [mount_bindfs](https://github.com/Halo-Michael/bindfs)
```
DYLD_INSERT_LIBRARIES=/var/jb/usr/lib/libdopaminekerneldo.dylib mount_bindfs -o ro /bin /cores

```
