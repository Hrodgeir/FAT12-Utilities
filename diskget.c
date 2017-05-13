/*

Get a file on disk.

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
	
	/* Check to see that the file exists. */
	int fileAddress = getFileAddress(p, argv[2]);
	if (fileAddress == -1)
	{
		printf("File not found.\n");
		exit(1);
	}

	/* Open the new file. */
	FILE* fp;
	if ((fp = fopen(argv[2], "w")) == NULL)
    {
    	printf("Opening the new file failed.\n");
    	exit(1);
    }

	/* Get file size and first logical cluster. */
	int fileSize = getFileSize(p, fileAddress);
	int firstLogicalCluster = getFirstLogicalCluster(p, fileAddress);
	int bytesPerSector = getBytesPerSector(p);

	char* fileData = malloc(sizeof(char));
	if (fileData == 0)
	{
		printf("Malloc failed.\n");
		exit(1);
	}
	
	/* Get the data of the first logical cluster. */
	int offset = (firstLogicalCluster + 31) * bytesPerSector;
	for (int l = 0; l < bytesPerSector; l++)
	{
		if (l == fileSize)
		{
			break;
		}
		
		fileData[l] = p[offset + l];
	}

	/* Calculate the location of the next logical cluster. */
	int k = firstLogicalCluster;
	int value = getNextLogicalCluster(p, k);

	/* Loop until there is no more data to retrieve. */
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

			fileData[l+bytesPerSector*count] = p[offset + l];
		}
		
		k++;
		value = getNextLogicalCluster(p, k);
		count++;
	}

    if (fwrite(fileData, 1, fileSize, fp) == EOF)
    {
    	printf("File writing failed.\n");
        exit(1);
    }

    /* Unmap memory and close files. */
    if (munmap(p, sf.st_size) == -1)
    {
        printf("Unmapping memory failed.\n");
        exit(1);
    }

    if (fclose(fp) == EOF)
    {
        printf("File closing failed.\n");
        exit(1);
    }

    if (close(fd) == -1)
    {
        printf("File closing failed.\n");
        exit(1);
    }

	return 0;
}