#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("USage: argument (filename) expected\n");
		exit(1);
	}

	DIR *dirPtr;
	struct dirent *entryPtr;
	dirPtr = opendir (argv[1]);
	
	struct stat statBuf;
	
	while ((entryPtr = readdir (dirPtr))) {
		
		if (stat (entryPtr->d_name, &statBuf) < 0) {
			perror ("Program error: ");
			exit(1);
		}
	
		printf("%-10s", entryPtr->d_name);
		printf("%d bytes\t", statBuf.st_size);
		printf("%d (Inode)\t", statBuf.st_ino);
		printf("%d|%d (UID|GID)\n", statBuf.st_uid, statBuf.st_gid);
	}
		
	closedir (dirPtr);
	return 0;
}
