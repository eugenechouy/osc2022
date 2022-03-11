#!/usr/bin/python3

import sys
import serial
import time

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: sendimg.py <img> <serial device file>")
        exit(1)
    
    try:
        ser = serial.Serial(sys.argv[2], 115200)
    except serial.SerialException as e:
        print("Serial init failed: ", e)
        exit(1)

    checksum = 0
    size     = 0
    img      = b''
    with open(sys.argv[1], 'rb') as f:
        while True:
            byte = f.read(1)
            if not byte:
                break
            img      += byte
            size     += 1
            checksum += int.from_bytes(byte, byteorder='big')
    
    print("Sent image size: ", size)
    print("Sent image checksum: ", checksum)
    ser.write(size.to_bytes(4, byteorder='big'))
    ser.write(checksum.to_bytes(4, byteorder='big'))
    ser.flush()

    # send size, checksum successfully
    print("Received image size: ", ser.readline().decode("utf-8"), end="")
    print("Received image checksum: ", ser.readline().decode("utf-8"), end="")

    time.sleep(3)

    chunk_size = 128
    chunk_cnt  = -(-size//chunk_size)
    for i in range(chunk_cnt):
        ser.write(img[i*chunk_size: (i+1)*chunk_size])
        ser.flush()

    # send image successfully
    print(ser.readline().decode("utf-8"), end="")

    ser.close()