/*

CSC 360 Assignment 3 Part 4
---------------------------
Matthew Hodgson
V00803081

*/

/* Built in C libraries. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>

/* Custom Disk Utilities. */
#include "diskutils.h"

int main(int argc, char* argv[])
{
	/* Error handling and file reader. */
    if (argc != 3)
    {
        printf("Diskget requires three arguments.\n");
        exit(1);
    }

    /* Open the disk image. */
    int pfd;
    if ((pfd = open(argv[1], O_RDWR)) == -1)
    {
    	printf("Opening the disk image failed.\n");
    	exit(1);
    }

    /* Map the disk image to a char array. */
	struct stat psf;
	if (fstat(pfd, &psf) == -1)
    {
        printf("Fstat Failed.\n");
        exit(1);
    }

	char* p = mmap(NULL, psf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, pfd, 0);
	if (p == MAP_FAILED)
    {
        printf("Disk image mapping failed: %d, %d\n", errno, (int)psf.st_size);
        exit(1);
    }

    /* Check to see that the file exists. */
	int fileAddress = getFileAddress(p, argv[2]);
	if (fileAddress != -1)
	{
		printf("File already exists on the disk image.\n");
		exit(1);
	}

    /* Open the file. */
    int qfd;
    if ((qfd = open(argv[2], O_RDONLY)) == -1) 
    {
    	printf("The specified file does not exist in the current directory.\n");
    	exit(1);
    }

    /* Map the file to a char array. */
    struct stat qsf;
	if (fstat(qfd, &qsf) == -1)
    {
        printf("Fstat Failed.\n");
        exit(1);
    }

    char* q = mmap(NULL, qsf.st_size, PROT_READ, MAP_SHARED, qfd, 0);
    if (q == MAP_FAILED)
    {
    	printf("File mapping failed.\n");
        exit(1);
    }

    int totalSectorCnt = getTotalSectorCount(p);
    int bytesPerSector = getBytesPerSector(p);
    int totalDiskSpace = totalSectorCnt * bytesPerSector;
    
    int availableDiskSpace = getFreeSize(p, totalDiskSpace);
    int fileSize = (int)qsf.st_size;

    if (availableDiskSpace < fileSize)
    {
    	printf("Not enough available space on the disk image.\n");
    	exit(1);
    }

    if (fileSize == 0)
    {
    	printf("File is empty, nothing to do.\n");
    	exit(1);
    }

    char* fileName = argv[2];

    int rootEntryAddress = createNewRootEntry(p, fileName, fileSize);

    /* Allocate space in the FAT Table. */
    int clustersRequired = fileSize / bytesPerSector + 1;
    int firstLogicalCluster = getFirstLogicalCluster(p, rootEntryAddress);

   	int k = firstLogicalCluster;

    for (int i = 1; i <= clustersRequired; i++)
    {
    	if (i == clustersRequired)
    	{
    		writeLogicalCluster(p, k, 1);
    		break;
    	}

    	writeLogicalCluster(p, k, 0);
    	k = getFreeLogicalCluster(p);
    }

    /* Write the data to the first logical cluster. */
	int offset = (firstLogicalCluster + 31) * bytesPerSector;
	for (int l = 0; l < bytesPerSector; l++)
	{
		if (l == fileSize)
		{
			break;
		}
		
		p[offset + l] = q[l];
	}

    /* Calculate the location of the next logical cluster. */
	k = firstLogicalCluster;
	int value = getNextLogicalCluster(p, k);

	/* Loop until there is no more data to write. */
	int count = 1;
	while((value & 0xFFF) < 0xFF7)
	{
		offset = (value + 31) * bytesPerSector; 
		for (int l = 0; l < bytesPerSector; l++)
		{
			if (l+bytesPerSector*count == fileSize)
			{
				break;
			}

			p[offset + l] = q[l+bytesPerSector*count];
		}	

		k++;
		value = getNextLogicalCluster(p, k);
		count++;
	}

	/* Unmap memory and close files. */
	if (munmap(p, psf.st_size) == -1)
    {
        printf("Unmapping memory failed.\n");
        exit(1);
    }

    if (munmap(q, qsf.st_size) == -1)
    {
        printf("Unmapping memory failed.\n");
        exit(1);
    }

    if (close(pfd) == -1)
    {
        printf("File closing failed.\n");
        exit(1);
    }

    if (close(qfd) == -1)
    {
        printf("File closing failed.\n");
        exit(1);
    }

	return 0;
}