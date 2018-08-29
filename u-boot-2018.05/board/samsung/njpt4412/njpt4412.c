// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Samsung Electronics
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mmc.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>
#include <usb.h>


#include <dm.h>
#include <power/pmic.h>
#include <mmc.h>
#include <usb/dwc2_udc.h>
#include <samsung/misc.h>

#include <config.h>
#include <asm/arch/dmc.h>


static void my_delay(unsigned int ms)
{
	while(ms--);
	return;
}

u32 get_board_rev(void)
{
	return 0;
}



/*
 * this will set the GPIO for hsmmc ch0
 * GPG0[0:6] = CLK, CMD, CDn, DAT[0:3]
 */
static void setup_hsmmc_cfg_gpio(void)
{
	ulong reg;

#ifdef USE_MMC0
	writel(0x02222222, 0x11000040);
	writel(0x00003FFC, 0x11000048);
	writel(0x00003FFF, 0x1100004c);
	writel(0x03333000, 0x11000060);
	writel(0x00003FC0, 0x11000068);
	writel(0x00003FC0, 0x1100006c);	
#endif

#ifdef USE_MMC1
#endif

#ifdef USE_MMC2
	writel(0x02222222, 0x11000080);
	writel(0x00003FF0, 0x11000088);
	writel(0x00003FFF, 0x1100008C);
#endif

#ifdef USE_MMC3
#endif

#ifdef USE_MMC4
	
	reg = (readl(0x11000048) & ~(0xf));
	writel(reg,0x11000048); //SD_4_CLK/SD_4_CMD pull-down enable


	reg = (readl(0x11000040) & ~(0xff));
	writel(reg,0x11000040);//cdn set to be output



	reg = (readl(0x11000048) & ~(3 << 4));
	writel(reg,0x11000048); //cdn pull-down disable 



	reg = (readl(0x11000044) & ~(1 << 2));
	writel(reg,0x11000044); //cdn output 0 to shutdown the emmc power



	reg = (readl(0x11000040) & ~(0xf << 8));
	reg |= (1 << 8);
	writel(reg,0x11000040);//cdn set to be output



	udelay(100*1000);


	reg = (readl(0x11000044) | (1 << 2));
	writel(reg,0x11000044); //cdn output 1



	writel(0x03333133, 0x11000040);
	writel(0x00003FF0, 0x11000048);
	writel(0x00002AAA, 0x1100004C);
	
#ifdef CONFIG_EMMC_8Bit
	writel(0x04444000, 0x11000060);
	writel(0x00003FC0, 0x11000068);
	writel(0x00002AAA, 0x1100006C);
#endif /*END CONFIG_EMMC_8Bit*/
	
#endif

}




int exynos_init(void)
{
	setup_hsmmc_cfg_gpio();
#if CONFIG_CMD_USB
	gpio_request(EXYNOS4X12_GPIO_X23,"USB3503A RefFreq");
	gpio_request(EXYNOS4X12_GPIO_M33,"USB3503A Connect");
	gpio_request(EXYNOS4X12_GPIO_M24,"USB3503A Reset");

	gpio_request(EXYNOS4X12_GPIO_C01,"DM9621 Reset");

#endif
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{

#if CONFIG_CMD_USB
	unsigned int value = 0;

	gpio_direction_output(EXYNOS4X12_GPIO_X23,1); //USB3503A_EXT19
	gpio_direction_output(EXYNOS4X12_GPIO_M33,0); //USB3503A_CONNECT
	gpio_direction_output(EXYNOS4X12_GPIO_M24,0); //USB3503A_REST
	gpio_direction_output(EXYNOS4X12_GPIO_M24,1); //USB3503A_REST
	gpio_direction_output(EXYNOS4X12_GPIO_M33,1); //USB3503A_CONNECT

	gpio_direction_output(EXYNOS4X12_GPIO_C01,0);

	value = readl(0x11000c40);
	value |= (0xf << 12);
	writel(value, 0x11000c40);  //GPMX2CON --- USB3503A_EXT19


	value = readl(0x110002a0);
	value |= (1 << 16);
	writel(value, 0x110002a0);  //GPM2CON --- USB3503A_REST

	value = readl(0x110002a4);
	value &= ~(1 << 4);
	writel(value, 0x110002a4);  //GPM2DAT --- USB3503A_REST
	debug("USB3503A_REST GPM2_4........ \n");

	value = readl(0x110002c0);
	value |= (1 << 12);
	writel(value, 0x110002c0);  //GPM3CON --- USB3503A_CONNECT

	value = readl(0x110002c4);
	value &= ~(1 << 3);
	writel(value, 0x110002c4);  //GPM3DAT ---USB3503A_CONNECT
	debug("USB3503A_CONNECT GPM3_3..... \n");


//	gpio_direction_output(EXYNOS4X12_GPIO_C01,1);
//	gpio_set_value(EXYNOS4X12_GPIO_C01,0);
	value = readl(0x11400064);
	value &= ~(1 << 1);
	writel(value, 0x11400064);  //GPC0DAT   C0_1 DM9621_RESET
	debug("dm9621 reset operator..... \n");

//	mdelay(100);
	my_delay(500000);


	value = readl(0x110002a4);
	value |= (1 << 4);
	writel(value, 0x110002a4);  //GPM2DAT --- USB3503A_REST


	value = readl(0x110002c4);
	value |= (1 << 3);
	writel(value, 0x110002c4);  //GPM3DAT ---USB3503A_CONNECT

//	writel((1 << 16), 0x110000C0);  //GPL0CON
//	writel((1 << 4),  0x110000C4);  //GPL0DAT
//	debug("4.3LCD enable  operator..... \n");
#endif
	return 0;
}


#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
/*
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_platname("VDD_UOTG_3.0V", &dev);
	if (ret) {
		error("Regulator get error: %d", ret);
		return ret;
	}

	if (on)
		return regulator_set_mode(dev, OPMODE_ON);
	else
		return regulator_set_mode(dev, OPMODE_LPM);
*/
	return 0;
}

struct dwc2_plat_otg_data s5pc210_otg_data = {
	.phy_control	= s5pc210_phy_control,
	.regs_phy	= EXYNOS4X12_USBPHY_BASE,
	.regs_otg	= EXYNOS4X12_USBOTG_BASE,
	.usb_phy_ctrl	= EXYNOS4X12_USBPHY_CONTROL,
	.usb_flags	= PHY0_SLEEP,
};
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int exynos_early_init_f(void)
{
	debug("USB_udc_probe\n");
	board_usb_init(0,0);

//	return dwc2_udc_probe(&s5pc210_otg_data);
    return 0;
}
#endif
