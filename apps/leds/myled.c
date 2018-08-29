#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	int fd;
	int val = 1;

	fd = open("/dev/led0",O_RDWR);
	if(fd < 0){
		printf("can not open! \n");
		return -1;
	}

	while(1){
		val= 0;
		write(fd,&val,1);
		usleep(500000);
		val = 0;
		write(fd,&val,1);
		usleep(500000);
	}
}
