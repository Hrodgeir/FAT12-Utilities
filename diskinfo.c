/*

Disk information.

*/

/* Built in C libraries. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Custom Disk Utilities. */
#include "diskutils.h"

int main(int argc, char* argv[])
{
	/* Error handling. */
    if (argc != 2)
    {
        printf("Diskinfo requires two arguments.\n");
        exit(1);
    }

    char* osName = malloc(sizeof(char));
    if (osName == 0)
    {
    	printf("Malloc failed.\n");
    	exit(1);
    }

    char* label = malloc(sizeof(char));
	if (label == 0)
    {
    	printf("Malloc failed.\n");
    	exit(1);
    }

    /* Open the disk image. */
    int fd;
    if ((fd = open(argv[1], O_RDONLY)) == -1) 
    {
    	printf("Opening the disk image failed.\n");
    	exit(1);
    }

    /* Map the disk image to a char array. */
    struct stat sf;
	if (fstat(fd, &sf) == -1)
    {
        printf("Fstat Failed.\n");
        exit(1);
    }

	char* p = mmap(NULL, sf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
    {
        printf("Disk image mapping failed.\n");
        exit(1);
    }

    /* Get the disk information. */
	int totalSectorCnt = getTotalSectorCount(p);
	int bytesPerSector = getBytesPerSector(p);

	getOSName(p, osName);
	getLabel(p, label);

	int totalSize = totalSectorCnt * bytesPerSector;
	int freeSize = getFreeSize(p, totalSize);
	int rootFileCount = getRootFileCount(p);
	int FATCopies = getFATCopies(p);
	int FATSectors = getFATSectors(p);
	
    /* Print the disk information. */
	printf("OS Name: %s\n", osName);
	printf("Label of the disk: %s\n", label);
	printf("Total size of the disk: %i bytes\n", totalSize);
	printf("Free size of the disk: %i bytes\n", freeSize);
	printf("==============\n");
	printf("The number of files in the root directory (not including subdirectories): %d\n", rootFileCount);
	printf("==============\n");
	printf("Number of FAT copies: %i\n", FATCopies);
	printf("Sectors per FAT: %i\n", FATSectors);

    /* Unmap memory and close files. */
    if (munmap(p, sf.st_size) == -1)
    {
        printf("Unmapping memory failed.\n");
        exit(1);
    }

    if (close(fd) == -1)
    {
        printf("File closing failed.\n");
        exit(1);
    }

	return 0;
}