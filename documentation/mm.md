
## memory layout

```
+------------------+
| Arm Peripherals  |
+------------------+ <- 0x40000000
| GPU Peripherals  |
+------------------+ <- 0x3f000000
|     reserved     |
+------------------+ <- 0x3C000000
|        .         |
|    User stack    |
|        .         |
+------------------+
|  Kernel frame[n] | <- statically declared
+------------------+
|        .         |
|        .         |
+------------------+
|  Kernel frame[0] |
+------------------+ 
|     Int stack    | <- statically declared
+------------------+
| kernel/user data |
+------------------+
| kernel/user text |
+------------------+ <- kernel_base
```