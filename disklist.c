/*

Listing what is on the disk.

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
        printf("Disklist requires two arguments.\n");
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

	int bytesPerSector = getBytesPerSector(p);
	int rootAddress = 19 * bytesPerSector;
	int rootSize = 14 * bytesPerSector;

	/* Loop and print each file and directory. */
	for (int i = rootAddress; i < rootAddress + rootSize; i += 32)
	{
		int attribute = getFileAttribute(p, i);
		if (attribute != -1 && (int)p[i] != 0)
		{
			/* Get the type, size, and name. */
			char type = attribute == 0 ? 'D' : 'F';
			int fileSize = getFileSize(p, i);
			char* fileName = getFileName(p, i);

			/* Get the creation time. */
			int creationTime = (int)((p[i + 14] & 0xFF) | (p[i + 15] & 0xFF) << 8);
			int hour = creationTime >> 11;
			int minute = (creationTime >> 5) & 0x3F;
			int second =  (creationTime & 0x1F) * 2;

			/* Get the creation date. */
			int creationDate = (int)((p[i + 16] & 0xFF) | (p[i + 17] & 0xFF) << 8);
			int year = 1980 + (creationDate >> 9);
			int month = (creationDate >> 5) & 0x1F;
			int day = creationDate & 0x1F;

			/* Format and print the file/directory information. */
			printf("%c ", type);
			printf("%d ", fileSize);
			printf("%s ", fileName);
			printf("%d/%02d/%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
		}
	}

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