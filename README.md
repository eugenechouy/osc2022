# My OSC 2022

Self-made operating system kernel running on Raspberry Pi 3 Model B+.

## Requirement

* gcc-aarch64-linux-gnu
* qemu-system-aarch64

## To Run

Create initramfs first.
``` bash
$ cd rootfs
$ make
$ make archv
```

### Emulator

Run kernel directly.
``` bash
$ make run
```

Or you can start from bootloader, then send the kernel image to bootloader.
``` bash
$ cd boot
$ make
$ make pty
...
# open another shell
$ make
$ python3 sendimg.py kernel8.img /dev/pts/1
$ screen /dev/pts/1 115200
```