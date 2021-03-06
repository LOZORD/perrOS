# Project 5: FS Bug Hunt

## FSCK for xv6 images
* Don't just do a diff
* Most functions will involve enumerations
* A lot of data collection and analysis
* Real FSCK has documentation of check phases
* The program runs _without_ user input
* We have to learn what the structure are in the xv6 file system
* What an inode looks like, what the superblock looks like..

### FSCK Steps
* Enumerate free blocks
* Enumerate inodes
* Enumerate inode contents (data blocks)
* Inode pointers should point to non-free data
* It is __corrupted__ if an inode points to free data
* In this case, mark the inode as invalid (and recurse the directory tree)
* Make sure inodes don't point to the same block
* Be sure to examine _single_ indirect blocks
* We don't want any indirect blocks in the free list
* We don't care what the type of a file is. __Don't look inside data blocks__
* Be sure to check the superblock as well (sniff test)
  * Compare number of inodes * `sizeof inode` and total memory
  * Make sure values are valid (that we can tell)
* For directories
  * We know something is a directory because the inode says so
  * Examine _those_ data block contents
  * Make sure `.` and `..` are there (with correct corresponding inode numbers)
  * Make sure we don't have any orphan directories (children w/o parents)
  * If we do, put it in `/lost+found`
  * We find names in directories
* If no inode references a valid data block, but it's not in the free list, just add it to the free list
* If the FS boots/mounts properly, it works

## Parsing LFS images
* We're just going to have one checkpoint region
* We don't have the idea of segments
* CR has ref to imaps (chunked throughout FS)
* Given an inode number:
  * First check out CR for correct imap
  * Use the inode number as key to the imap

### `lfsreader arg=[cat | ls] path fsimage`
* `cat`
  * prints out the contents of the file at that path
  * check intermediate directories for validity
  * A lot of reading and seeking
* `ls`
  * Think of it as a cat for directories
  * Just print file and directory names
  * Do error checking, but then we don't have to fix them
  * Just report the error if any

# Second day of discussion

A file system is a non-human readable serialization.

`fsck` finds problems and fixes them.

```c
struct superblock {
  uint size;
  unit numinodes;
  uint numblocks;
};
```

## Two methods for getting the image into memory
* `lseek`, `read`, `write`
  * treats image as a datafile
  * read in bytes like a file
  * Buffer superblock, inodes, blocks in to memory from "file"
* `mmap`
  * the file gets placed in memory
  * Perry's opinion is that `lseek` should be used
  * Remember to copy it back to disk

## Tests
* See if we can run our image if there are no irrecoverable errors
* Can we mount the fs image?

## LFS
We don't write to the disk at all. All we do is read.
Start by going to the data structures.

# Sankar's Checks
* Superblock
  * num blocks > size
  * num inodes > size
* Inodes
  * valid type
  * file/directory size > fs size
  * sensible ref count
  * all inodes are reachable from root (every inode should have a parent [except root])
* Data blocks
  * No block should be pointed to by more than one inode
  * Inode check is complete (you should know the data blocks reference valid inodes)
  * Compare this against free data block bitmap
  * Fix any inconsistencies
* Directories
  * Check that `.` and `..` references exist and are valid
  * Clear directory entry if invalid
  * Place orphans in `lost+found`
    * Valid inodes with no reference
    * Give some random, UUID
