#!/bin/bash

find . | cpio -o -H newc > ../initramfs.cpio