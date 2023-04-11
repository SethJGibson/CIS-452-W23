#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int main()
{
	DIR *dirPtr;
	struct dirent *entryPtr;
	dirPtr = opendir (".");
	
	struct stat statBuf;
	
	while ((entryPtr = readdir (dirPtr))) {
		
		if (stat (entryPtr->d_name, &statBuf) < 0) {
			perror ("Program error: ");
			exit(1);
		}
	
		printf ("%-20s", entryPtr->d_name);
		printf("%d bytes\n", statBuf.st_size);
	}
		
	closedir (dirPtr);
	return 0;
}
