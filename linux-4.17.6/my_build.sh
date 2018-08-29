#!/bin/bash 

make dtbs

make uImage LOADADDR=0x40008000

cp -a arch/arm/boot/uImage /mnt/hgfs/share/exynos4412/led/ 

echo "arch/arm/boot/uImage /mnt/hgfs/share/exynos4412/led/"

cp -a arch/arm/boot/dts/exynos4412-itop-elite.dtb /mnt/hgfs/share/exynos4412/led/ 

echo "arch/arm/boot/dts/exynos4412-itop-elite.dtb /mnt/hgfs/share/exynos4412/led/"


