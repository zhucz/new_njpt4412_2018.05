#!/bin/sh

sec_path="./CodeSign4SecureBoot"
ROOT_DIR=$(pwd)

rm -rf u-boot.bin
make -j$CPU_JOB_NUM

if [ ! -f u-boot.bin ]
then
	echo "!!!not found u-boot.bin"
fi

cp spl/njpt4412-spl.bin bl2.bin

####################################################
#cat spl/u-boot-spl.bin pad00.bin > image.bin

#./mkbl2 image.bin bl2.bin 14336
####################################################

cp -rf bl2.bin $sec_path
cp -rf u-boot.bin $sec_path
cd $sec_path


echo -e "\033[32m fusing u-boot-njpt4412.bin...... \033[0m"

################ for sd MMC boot ##################
# cat E4412_N.bl1.SCP2G.bin bl2.bin env.bin u-boot.bin > u-boot-njpt4412.bin
cat E4412_N.bl1.bin bl2.bin env.bin u-boot.bin > u-boot-njpt4412.bin
################# for eMMC boot ####################
#cat E4412_N.bl1.SCP2G.bin bl2.bin u-boot.bin > u-boot-njpt4412.bin
####################################################
mv u-boot-njpt4412.bin $ROOT_DIR
cd $ROOT_DIR

echo "done!!!"
