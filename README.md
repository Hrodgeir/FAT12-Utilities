# FAT12Utilities
Various utilities for FAT12 file system including getting disk information, what is stored on a disk image, and writing to a disk image.

----------------------------------------------------------
Compiling diskinfo.c, disklist.c, diskget.c and diskput.c

To compile the four programs, place the attached Makefile 
in the same directory as each program as well as the
diskutils.h file.

Type "make" in the command line, this will compile all
four of the programs.

----------------------------------------------------------
Running diskinfo.c

To run diskinfo.c, type "./diskinfo disk.IMA" in the 
command line.

----------------------------------------------------------
Running disklist.c

To run disklist.c, type "./disklist disk.IMA" in the 
command line.

----------------------------------------------------------
Running diskget.c

To run diskget.c, type "./diskget disk.IMA file.txt" in 
the command line, where file.txt is the file and extension 
that you would like to retrieve.

----------------------------------------------------------
Running diskput.c

To run diskput.c, type "./diskput disk.IMA file.txt" in 
the command line, where file.txt is the file and extension 
that you would like to write to the disk image.
