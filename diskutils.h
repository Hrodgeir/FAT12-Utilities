/*

CSC 360 Assignment 3 Header File
--------------------------------
Matthew Hodgson
V00803081

*/

/* Built in C libraries. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Function Prototypes. */
void getOSName(char* p, char* osName);
void getLabel(char* p, char* label);
void removeSpaces(char* source);
void writeLogicalCluster(char* p, int c, int lastCluster);
int createNewRootEntry(char* p, char* fileName, int fileSize);
int getTotalSectorCount(char* p);
int getBytesPerSector(char* p);
int getFATCopies(char* p);
int getFATSectors(char* p);
int getFreeSize(char* p, int totalSize);
int getFileAddress(char* p, char* targetFile);
int getRootFileCount(char* p);
int getFreeRootEntryAddress(char* p);
int getFileSize(char* p, int i);
int getFirstLogicalCluster(char* p, int i);
int getNextLogicalCluster(char* p, int i);
int getFreeLogicalCluster(char* p);
char* getFileName(char* p, int i);
int* getCurrentTime();

/* Function Defitions. */
void getOSName(char* p, char* osName)
{
	int offset = 3;
	for (int i = 0; i < 8; i++)
	{
		osName[i] = p[i + offset];
	}
}

void getLabel(char* p, char* label)
{
	int bytesPerSector = getBytesPerSector(p);
	int rootAddress = 19 * bytesPerSector;
	int rootSize = 14 * bytesPerSector;

	int offset = 0;
	for (int i = rootAddress + 11; i < rootAddress + rootSize; i += 32)
	{
		if ((int)p[i] == 8)
		{
			offset = i - 11;
			break;
		}
	}

	for (int i = 0; i < 12; i++)
	{
		label[i] = p[i + offset];
	}
}

void removeSpaces(char* source)
{
	char* i = source;
	char* j = source;
	while(*j != 0)
	{
		*i = *j++;
		if (*i != ' ')
		{
			i++;
		}
	}
	*i = 0;
}

void writeLogicalCluster(char* p, int c, int lastCluster)
{
	int bytesPerSector = getBytesPerSector(p);
	int hi = 1+(3*c)/2 + bytesPerSector;
	int lo = (3*c)/2 + bytesPerSector;

	if (c % 2 == 0)
	{
		if (lastCluster)
		{
			p[hi] = 0x0F;
			p[lo] = 0xFF;
		}
		else
		{
			p[hi] = p[hi] | ((((c + 1) & 0xFF) >> 8) & 0x0F);
			p[lo] = p[lo] | ((c + 1) & 0xFF);
		}
	}
	else
	{
		if (lastCluster)
		{
			p[hi] = 0xFF;
			p[lo] = 0xF0;
		}
		else
		{
			p[hi] = p[hi] | (((c + 1) & 0xFF) >> 4);
			p[lo] = p[lo] | ((((c + 1) & 0xFF) << 4) & 0xF0);
		}
	}
}

int createNewRootEntry(char* p, char* fileName, int fileSize)
{
	int rootEntryAddress = getFreeRootEntryAddress(p);

    /* Write the file name to root entry. */
    removeSpaces(fileName);
    char* name = strtok(fileName, ".");
    char* extension = strtok(NULL, ".");

    if (strlen(name) > 8)
    {
    	printf("Specified file name is too long.\n");
    	exit(1);
    }

    if (strlen(extension) > 3)
    {
    	printf("Specified extension is too long.\n");
    	exit(1);
    }

    for (int i = 0; i < 8; i++)
    {
    	p[rootEntryAddress+i] = i < strlen(name) ? name[i] : ' ';
    }

    for (int i = 8; i < 11; i++)
    {
    	p[rootEntryAddress+i] = i < strlen(extension) + 8 ? extension[i-8] : ' ';
    }

    /* Write the file creation time and date to root entry. */
    int* creationTime = getCurrentTime();
    int year = creationTime[0] - 80;
    int month = creationTime[1] + 1;
    int day = creationTime[2];
    int hour = creationTime[3];
    int minute = creationTime[4];
    int second = creationTime[5];

	int timeToWrite = ((hour << 11) | (minute << 5) | (second / 2));
	p[rootEntryAddress+14] = timeToWrite & 0xFF;
	p[rootEntryAddress+15] = timeToWrite >> 8;
	p[rootEntryAddress+22] = timeToWrite & 0xFF;
	p[rootEntryAddress+23] = timeToWrite >> 8;

	int dateToWrite = ((year << 9) | (month << 5) | day);
	p[rootEntryAddress+16] = dateToWrite & 0xFF;
	p[rootEntryAddress+17] = dateToWrite >> 8;
	p[rootEntryAddress+24] = dateToWrite & 0xFF;
	p[rootEntryAddress+25] = dateToWrite >> 8;

	/* Write the address of the first cluster to the root entry. */
	int firstLogicalCluster = getFreeLogicalCluster(p);

	p[rootEntryAddress+26] = firstLogicalCluster & 0xFF;
	p[rootEntryAddress+27] = firstLogicalCluster >> 8;

	/* Write the file size to the root entry. */
	p[rootEntryAddress+28] = fileSize & 0xFF;
	p[rootEntryAddress+29] = fileSize >> 8;
	p[rootEntryAddress+30] = fileSize >> 16;
	p[rootEntryAddress+31] = fileSize >> 24;

	return rootEntryAddress;
}

int getTotalSectorCount(char* p)
{
	return (int) (p[19] | p[20] << 8);
}

int getBytesPerSector(char* p)
{
	return (int) (p[11] | p[12] << 8);
}

int getFATCopies(char* p)
{
	return (int) p[16];
}

int getFATSectors(char* p)
{
	return (int) p[22];
}

int getFileAttribute(char* p, int address)
{
	int attributeAddress = address + 11;
	int attribute = p[attributeAddress] & 0xFF;

	if ((attribute & 0x0F) == 0x0F || (attribute & 0x08) == 0x08)
	{
		return -1;
	}
	else if (attribute & 0x10 == 0x10)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int getFreeSize(char* p, int totalSize)
{
	int usedSectors = 0;

	int bytesPerSector = getBytesPerSector(p);
	int FATTableAddress = bytesPerSector;
	int FATTableSize = 9 * bytesPerSector;

	/* Iterate through FAT table and find used sectors. */
	for (int i = FATTableAddress; i < FATTableSize; i++)
	{
		int value = getNextLogicalCluster(p, i);

		if (value != 0)
		{
			usedSectors++;
		}
	}

	/* Calculate the free size on the disk. */
	/* Remove 1 sector to account for boot directory. */
	int usedSize = usedSectors * bytesPerSector;
	return totalSize - usedSize;
}

int getFileAddress(char* p, char* targetFile)
{
	int bytesPerSector = getBytesPerSector(p);
	int rootAddress = 19 * bytesPerSector;
	int rootSize = 14 * bytesPerSector;

	for (int i = rootAddress; i < rootAddress + rootSize; i += 32)
	{
		int attribute = getFileAttribute(p, i);
		if (attribute == 1 && (int)p[i] != 0)
		{
			char* fileName = getFileName(p, i);

			if (strcmp(targetFile, fileName) == 0)
			{
				return i;
			}
		}
	}
	
	return -1;
}

int getRootFileCount(char* p)
{
	int rootFileCount = 0;

	int bytesPerSector = getBytesPerSector(p);
	int rootAddress = 19 * bytesPerSector;
	int rootSize = 14 * bytesPerSector;

	for (int i = rootAddress; i < rootAddress + rootSize; i += 32)
	{
		int value = getFileAttribute(p, i);
		if (value == 1 && (int)p[i] != 0)
		{
			rootFileCount++;
		}
	}

	return rootFileCount;
}

int getFreeRootEntryAddress(char* p)
{
	int bytesPerSector = getBytesPerSector(p);
	int rootAddress = 19 * bytesPerSector;
	int rootSize = 14 * bytesPerSector;

	for (int i = rootAddress; i < rootAddress + rootSize; i += 32)
	{
		if ((int)p[i] == 0)
		{
			return i;
		}
	}

	return -1;
}

int getFileSize(char* p, int i)
{
	return (int)((p[i + 28] & 0xFF) 
		| ((p[i + 29] & 0xFF) << 8) 
		| ((p[i + 30] & 0xFF) << 16) 
		| ((p[i + 31] & 0xFF) << 24));
}

int getFirstLogicalCluster(char* p, int i)
{
	return (int)((p[i + 26] & 0xFF) | (p[i + 27] & 0xFF) << 8);
}

int getNextLogicalCluster(char* p, int k)
{
	int bytesPerSector = getBytesPerSector(p);
	int hi = 1+(3*k)/2 + bytesPerSector;
	int lo = (3*k)/2 + bytesPerSector;
	if (k % 2 == 0)
	{
		return (int)((((p[hi] & 0xFF) & 0x0F) << 8) | (p[lo] & 0xFF));
	}
	else
	{
		return (int)(((p[hi] & 0xFF) << 4) | (((p[lo] & 0xFF) & 0xF0) >> 4));
	}
}

int getFreeLogicalCluster(char* p)
{
	int bytesPerSector = getBytesPerSector(p);
	int FATTableAddress = bytesPerSector;
	int FATTableSize = 9 * bytesPerSector;

	for (int i = 0; i < FATTableSize-512; i++)
	{
		int value = getNextLogicalCluster(p, i);

		if (value == 0)
		{
			return i;
		}
	}
}

char* getFileName(char* p, int i)
{
	char* fileName = malloc(sizeof(char));
	if (fileName == 0)
	{
		printf("Malloc failed.\n");
		exit(1);
	}

	for (int j = 0; j < 8; j++)
	{
		fileName[j] = p[i + j];
	}

	fileName[8] = '.';

	for (int j = 9; j < 12; j++)
	{
		fileName[j] = p[i + j - 1];
	}

	removeSpaces(fileName);

	return fileName;
}

int* getCurrentTime()
{
	time_t now;
	struct tm* ts;
	now = time(NULL);
	ts = localtime(&now);

	int* currentTime = malloc(sizeof(int));
	if (currentTime == 0)
	{
		printf("Malloc failed.\n");
		exit(1);
	}

	currentTime[0] = ts->tm_year;
	currentTime[1] = ts->tm_mon;
	currentTime[2] = ts->tm_mday;
	currentTime[3] = ts->tm_hour;
	currentTime[4] = ts->tm_min;
	currentTime[5] = ts->tm_sec;

	return currentTime;
}