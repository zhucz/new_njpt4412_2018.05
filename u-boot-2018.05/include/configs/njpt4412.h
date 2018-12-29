/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG ORIGEN (EXYNOS4210) board.
 */

#ifndef __CONFIG_NJPT4412_H
#define __CONFIG_NJPT4412_H

#include <configs/exynos4-common.h>

/* #define DEBUG */

/*EMMC */
#define USE_MMC2  
#define USE_MMC4  
/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC




#define CONFIG_DEBUG_UART
#define CONFIG_SPL_SERIAL_SUPPORT

#define CONFIG_S5P
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_DEBUG_UART_S5P
#define CONFIG_DEBUG_UART_BASE   	0x13820000    /* UART2--SERIAL3 base address  */
#define CONFIG_DEBUG_UART_CLOCK 	(100000000)    /* SCLK_UART3 is 100MHz  */

/* High Level Configuration Options */
#define CONFIG_EXYNOS4210		1	/* which is a EXYNOS4210 SoC */
#define CONFIG_NJPT4412			1	/* working with ORIGEN*/

#define CONFIG_SYS_DCACHE_OFF		1

/* NJPT4412 has 8 bank of DRAM */
#define CONFIG_NR_DRAM_BANKS		8
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM_1			CONFIG_SYS_SDRAM_BASE
#define SDRAM_BANK_SIZE			(256 << 20)	/* 256 MB */

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x6000000)
/* #define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x3E00000) */ /*remove by zhuchengzhi */
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x00100000)


#define CONFIG_MACH_TYPE		MACH_TYPE_NJPT4412

/* select serial console configuration */
#define CONFIG_SERIAL2
#define CONFIG_BAUDRATE			115200 /*add by zhuchengzhi */


/* Console configuration */
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#define CONFIG_SYS_MEM_TOP_HIDE	(1 << 20)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

/* Power Down Modes */
#define S5P_CHECK_SLEEP			0x00000BAD
#define S5P_CHECK_DIDLE			0xBAD00000
#define S5P_CHECK_LPA			0xABAD0000

/* MMC SPL */
#define COPY_BL2_FNPTR_ADDR		0x02020030
/* #define CONFIG_SPL_TEXT_BASE	0x02021410 */  /*remove by zhuchengzhi */
#define CONFIG_SPL_TEXT_BASE	0x02023400	/*0x02021410*/ 


/*DM9601 add by zhuchengzhi 2018-06-03 TODO*/
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_GENERIC
#define CONFIG_USB_EHCI_EXYNOS					1
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS      3
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_HOST

#define CONFIG_USB_STORAGE


#define CONFIG_CMD_PING  1
#define CONFIG_CMD_NET
#define CONFIG_CMD_BOOTP
#define CONFIG_CMD_TFTPBOOT
#define CONFIG_CMD_TFTPPUT
#define CONFIG_CMD_TFTPSRV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
#define CONFIG_BOOTP_SERVERIP

#define CONFIG_USB_ETHER_DM9621 



/*USB HOST*/
#define CONFIG_CMD_USB  1
/*#define CONFIG_USB_EHCI_FSL*/
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET


#define CONFIG_USB_OHCI
/* #undef CONFIG_USB_STORAGE */
#define CONFIG_S3C_USBD
#define USBD_DOWN_ADDR		0xc0000000

#define USB_PHY_CONTROL_OFFSET		0x0704
#define USB_PHY_CONTROL            (0x10020000+USB_PHY_CONTROL_OFFSET)//(ELFIN_CLOCK_POWER_BASE+USB_PHY_CONTROL_OFFSET)

#define CONFIG_USB_OHCI_GENERIC
/*#define CONFIG_USB_XHCI_EXYNOS */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS   3
#define CONFIG_USB_EHCI_HCD

#define CONFIG_SYS_STDIO_DEREGISTER		1

#define CONFIG_IPADDR		192.168.1.103
#define CONFIG_SERVERIP		192.168.1.102 
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_BOOTARGS		"root=/dev/nfs rw nfsroot=192.168.1.104:/home/tigers/nfsroot/rootfs ip=192.168.1.103 init=/linuxrc console=ttySAC2,115200 earlyprintk"
#define CONFIG_BOOTCOMMAND	"usb start;tftpboot 41000000 uImage;tftpboot 42000000 exynos4412-itop-elite.dtb;bootm 41000000 - 42000000"


/*I2C*/
#define CONFIG_SYS_I2C_S3C24X0

/*
#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x40007000\0" \
	"rdaddr=0x48000000\0" \
	"kerneladdr=0x40007000\0" \
	"ramdiskaddr=0x48000000\0" \
	"console=ttySAC2,115200n8\0" \
	"mmcdev=0\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
        "loadbootscript=load mmc ${mmcdev} ${loadaddr} boot.scr\0" \
        "bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
                "source ${loadaddr}\0"

*/

/*#define CONFIG_BOOTCOMMAND \
	"if mmc rescan; then " \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"fi; " \
	"fi;" \
	"load mmc ${mmcdev} ${loadaddr} uImage; bootm ${loadaddr} "
*/


#define CONFIG_CLK_1000_400_200

/* MIU (Memory Interleaving Unit) */
#define CONFIG_MIU_2BIT_21_7_INTERLEAVED

#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(16 << 10)	/* 16 KB */
#define RESERVE_BLOCK_SIZE		(512)
#define BL1_SIZE			(16 << 10) /*16 K reserved for BL1*/
#define CONFIG_ENV_OFFSET		(RESERVE_BLOCK_SIZE + BL1_SIZE)

#define CONFIG_SPL_MAX_FOOTPRINT	(14 * 1024)

/* #define CONFIG_SYS_INIT_SP_ADDR		0x02040000 */

/*----------add by zhuchengzhi-------------*/

#define CONFIG_SPL_STACK			0x02040000 
#define UBOOT_SIZE					(2 << 20) /*0x0020 0000 = 2M */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + UBOOT_SIZE - 0x1000)
/*-----------------------------------------*/

/* U-Boot copy size from boot Media to DRAM.*/
#define COPY_BL2_SIZE		0x80000
#define BL2_START_OFFSET	((CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)/512)
#define BL2_SIZE_BLOC_COUNT	(COPY_BL2_SIZE/512)

#endif	/* __CONFIG_H */
