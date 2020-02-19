#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int fd;

	fd = open("/dev/testChar", O_RDWR);
	if(fd == -1){
		fprintf(stderr, "Open /dev/testChar failed \n");
		perror("open()");
		exit(1);
	}

	close(fd);
	exit(0);
}
