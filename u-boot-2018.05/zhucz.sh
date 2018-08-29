#!/bin/sh

# ### $1 =  /dev/sdb 

if [ -z $1 ]
then
    echo "usage: ./zhucz.sh.sh <SD Reader's device file>"
    exit 0
fi

echo -e "\033[32m### mkbl2 u-boot-spl.bin bl2.bin 14336 ### \033[0m"

#make clean

#make 

#sudo ./mkbl2 ./spl/u-boot-spl.bin bl2.bin 14336 


echo "\033[32m### sudo ./mkbl2 ./spl/u-boot-spl.bin bl2.bin 14336\033[0m"

sudo chmod 777 ./bl2.bin

echo "\033[32m### sudo chmod 777 bl2.bin \033[0m"

sudo dd if=../sd_fuse/tiny4412/E4412_N.bl1.bin of=$1 seek=1 iflag=dsync oflag=dsync

echo "\033[32m### sudo dd if=../sd_fuse/tiny4412/E4412_N.bl1.bin of=$1 seek=1 iflag=dsync oflag=dsync  [done]  \033[0m"

sudo dd if=./bl2.bin of=$1 seek=17 iflag=dsync oflag=dsync

echo "\033[32m###sudo dd if=./bl2.bin of=$1 seek=17 iflag=dsync oflag=dsync [done] \033[0m"

sudo dd if=./u-boot.bin of=$1 seek=65 iflag=dsync oflag=dsync

echo "\033[32m### sudo dd if=./u-boot.bin of=$1 seek=65 iflag=dsync oflag=dsync [done] \033[0m"

sync

echo "\033[32m### sync [done]\033[0m"

echo "\033[32m### Write To SD Card Successfull ###\033[0m"



