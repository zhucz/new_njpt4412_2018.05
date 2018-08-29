#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
	int fd;
	int val = 1;

	fd = open("/dev/mypwm1",O_RDWR);
	if(fd < 0){
		printf("can not open /dev/mypwm1! \n");
		return -1;
	}

	val= 0;
	ioctl(fd,val,1);

	return 0;
}
