#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
	int fd, i;
	unsigned char *buf = NULL;

	fd = open(argv[1], O_RDWR);
	if(fd == -1){
		perror("open()");
		return -1;
	}
	printf("fd is 0x%d \n", fd);

	buf = mmap(NULL, 8192, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(buf == NULL){
		perror("mmap()");
		exit(1);
	}

	printf("read from kernel : data is : \n");
	for(i = 0;i < 64;i++){
		printf("%x \t", buf[i]);
		if(i % 15 == 0)
			printf("\n");
	}

	printf("\n");

	for(i = 0;i < 64;i++)
		buf[i] = 0x55;

	printf("After write data is :\n");
	for(i = 0;i < 100;i++){
		printf("%x \t", buf[i]);
		if(i % 15 == 0)
			printf("\n");
	}

	printf("\n");

	close(fd);

	return 0;
	
}
