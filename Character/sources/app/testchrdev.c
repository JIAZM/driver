#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>	//ioctl()

static void my_exit(void)
{
	fprintf(stdout, "Get into my exit function from atexit \n");
}

int main(int argc, char *argv[])
{
	int fd, cmd, arg, len;
	char buf[4] = "123";
	char buf_rd[5] = {0};

	if(argc < 4){
		fprintf(stderr, "Usage... \n");
		fprintf(stderr, "testchrdev <device> <CmdNumber> <args> \n");
		exit(1);
	}
	atexit(my_exit);

	cmd = atoi(argv[2]);
	arg = atoi(argv[3]);
	fprintf(stdout, "cmd : %d, arg : %d \n", cmd, arg);

	fd = open(argv[1], O_RDWR);
	if(fd == -1){
		fprintf(stderr, "Open /dev/testChar failed \n");
		perror("open()");
		exit(1);
	}

/*
	ioctl(fd, 0, 0);
	ioctl(fd, 1, 0);
*/
	// ioctl(fd, cmd, arg);
	
	// fprintf(stdout, "buf contain : %s \n", buf);
	// len = write(fd, buf, 4);
	// fprintf(stdout, "write length : %d \n", len);

	len = read(fd, buf_rd, 5);
	fprintf(stdout, "Read length : %d \t Get string : %s \n", len, buf_rd);

	close(fd);
	exit(0);
}
