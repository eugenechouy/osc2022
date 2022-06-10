
## On-disk structure

```
+------------------+
|  Reserved Region |
+------------------+ <-- BPB_RsvdSecCnt
|    FAT Region    |
+------------------+ <-- BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz32)
|   Data Region    |
+------------------+
```

### Reserved Region

* Sector 0: The boot sector
    * BPB_BytsPerSec
    * BPB_SecPerClus
    * BPB_RootClus
* Sector 1: FSInfo sector (BPB_FSInfo)
* Sector 6: Copy of the boot sector (BPB_BkBootSec)

### FAT Region

* BPB_FATSz32: The number of sectors in each FAT.
* BPB_NumFATs: The number of FATs is stored in the boot sector.
* The FAT indicates which clusters are used and which are free. (For used clusters, the FAT indicates the next cluster that logically follows in a linked-list fashion)
* Each sector in the FAT cotains a list of **uint32_t** cluster numbers.

Each FAT entry indicates the state of that numbered cluster
* `FAT[cluster] == FAT_ENTRY_FREE`, cluster is free.
* `FAT[cluster] >= FAT_ENTRY_RESERVED_TO_END`, the cluster is used, which also be the last cluster in the file.
* `FAT[cluster] == FAT_ENTRY_DEFECTIVE_CLUSTER`, indicate defective cluster. 
* otherwise, the cluster is used, it contains the next cluster number in the file.

### Data Region

* The first file in the data region is the root directory

#### Directory

The first byte in a directory entry determines if the entry is either:
1. unused and to be skipped
2. unused and marks the end of the directory
3. used

If the entry is used, then the attribute byte is examined to determine if the entry is for a short filename or for a long filename.

Short Filename
* The first short filename character may not be a space (0x20)
* No lowercase characters may be in the short filename

#### File

