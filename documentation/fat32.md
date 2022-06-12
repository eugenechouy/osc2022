
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

#### FSInfo sector

* The FSInfo sector has two hints about free clusters
    * FSI_Nxt_Free
    * FSI_Free_Count

### FAT Region

* BPB_FATSz32: The number of sectors in each FAT.
* BPB_NumFATs: The number of FATs is stored in the boot sector.
* The FAT indicates which clusters are used and which are free. (For used clusters, the FAT indicates the next cluster that logically follows in a linked-list fashion)
* Each sector in the FAT cotains a list of **uint32_t** cluster numbers.

Each FAT entry indicates the state of that numbered cluster
* `FAT[cluster] == FAT_ENTRY_FREE`, cluster is free.
* `FAT[cluster] >= FAT_ENTRY_RESERVED_TO_END`, the cluster is used, which also be the last cluster in the file.
* `FAT[cluster] == FAT_ENTRY_DEFECTIVE_CLUSTER`, indicate defective cluster. 
* Otherwise, the cluster is used, it contains the next cluster number in the file.

> When writing the FAT entry for the last cluster in a file, use the value 0x0fffffff

### Data Region

* The first file in the data region is the root directory

#### Directory

The first byte in a directory entry determines if the entry is either:
1. unused (DIR_ENTRY_UNUSED)
2. unused and marks the end of the directory (DIR_ENTRY_LAST_AND_UNUSED)
3. used

If the entry is used, then the attribute byte is examined to determine if the entry is for a short filename or for a long filename.

Short Filename
* The first short filename character may not be a space (0x20)
* No lowercase characters may be in the short filename
* Terminate and fill both the main filename part and the extension name field with spaces(0x20)

Long Filename
* **Each long filename has an associated short filename entry** and that short filename entry must be the last entry for that group (continus directory entries)

To create a new entry in directory
1. Read through the directory to ensure name conflict doesn't exist.
2. Find either an unused directory entry or extend the directory.
    * To extend the directory: use the last entry and tag the next entry as FAT32_DIR_ENTRY_LAST_AND_UNUSED
    * If there is no more space in this cluster, then we need to find a free cluster, and update FAT

#### File

Files are allocated in units of clusters, which may be unused bytes(sectors) at the end of the last sector(cluster).

To extend a file
* DIR_FileSize must be update
* If the current cluster is full, an additional cluster needs to be allocated ...
