#!/bin/bash 

make uImage LOADADDR=0x40008000

cp -a arch/arm/boot/uImage /mnt/hgfs/share/exynos4412/led/ 



