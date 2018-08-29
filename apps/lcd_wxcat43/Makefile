
# Linux modules compile
# Author : zhucz
# Time   : 2018-8-28 10:34:05
#############################################################################
CC            = /usr/local/gcc-linaro-7.2.1-2017.11-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
CFLAGS        = -Wall -std=gnu99 -pthread
TARGET        = mylcd_rgb

OBJS          = $(TARGET).o

INSTALL_PATH  = ~/nfsroot/rootfs/bin/

$(TARGET) : $(OBJS)
	 $(CC) $(CFLAGS) -static -o $@ $<
	 rm -rf *.o

install:
	chmod 777 $(TARGET)
	cp $(TARGET) $(INSTALL_PATH) 

clean:
	rm -rf *.o $(TARGET) 
