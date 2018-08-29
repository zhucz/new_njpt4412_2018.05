/*
 * Clock Initialization for board based on EXYNOS4210
 *
 * Copyright (C) 2013 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#if 1 /* NJPT add by zhuchengzhi  */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include "common_setup.h"
#include "njpt4412_setup.h"

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
void system_clock_init(void)
{
	unsigned int set, clr, clr_src_cpu, clr_pll_con0, clr_src_dmc, value;
	unsigned int *addr;
	struct exynos4x12_clock *clk = (struct exynos4x12_clock *)samsung_get_base_clock();

	// src_cpu
	writel(0, &clk->src_cpu);
	while (readl(&clk->mux_stat_cpu) != 0x01110001)
		continue;

	set = ACP_RATIO(3) | ACP_PCLK_RATIO(1) | DPHY_RATIO(1)
		| DMC_RATIO(1) | DMCD_RATIO(1) | DMCP_RATIO(1) 
		| COPY2_RATIO(3) | CORE_TIMERS_RATIO(1);
	
	clr = ACP_RATIO(7) | ACP_PCLK_RATIO(7) | DPHY_RATIO(7)
		| DMC_RATIO(7) | DMCD_RATIO(7) | DMCP_RATIO(7) 
		| COPY2_RATIO(7) | CORE_TIMERS_RATIO(7);

	clrsetbits_le32(&clk->div_dmc0, clr, set);

	clr = C2C_ACLK_RATIO(0x7) | PWI_RATIO(0xf) | DVSEM_RATIO(0x7f) | DPM_RATIO(0x7f);
	set = PWI_RATIO(1) | DVSEM_RATIO(1) | DPM_RATIO(1);
	clrsetbits_le32(&clk->div_dmc1, clr, set);

	writel(0, &clk->src_top0);
	while (readl(&clk->mux_stat_top0) != 0x11111111)
		continue;

	writel(0, &clk->src_top1);
	while (readl(&clk->mux_stat_top1) != 0x01111110)
		continue;

	writel (CLK_DIV_TOP_VAL, &clk->div_top);
	
	writel (0x10, &clk->src_leftbus);
	while (readl(&clk->mux_stat_leftbus) != 0x21)
		continue;
	writel (CLK_DIV_LEFTBUS_VAL, &clk->div_leftbus);

	writel (0x10, &clk->src_rightbus);
	while (readl(&clk->mux_stat_rightbus) != 0x21)
		continue;

	writel (CLK_DIV_RIGHTBUS_VAL, &clk->div_rightbus);
	
#define APLL_LOCK_VAL	0x2F1
#define MPLL_LOCK_VAL	0x2F1
#define EPLL_LOCK_VAL	0x2321
#define VPLL_LOCK_VAL	0x2321
	writel(APLL_LOCK_VAL, &clk->apll_lock);
	writel(MPLL_LOCK_VAL, &clk->mpll_lock);
	writel(EPLL_LOCK_VAL, &clk->epll_lock);
	writel(VPLL_LOCK_VAL, &clk->vpll_lock);

	set = CORE_RATIO(0) | COREM0_RATIO(3) | COREM1_RATIO(7) | PERIPH_RATIO(3) | ATB_RATIO(4) | PCLK_DBG_RATIO(1) | APLL_RATIO(1) | CORE2_RATIO(0);
	
	clr = CORE_RATIO(7) | COREM0_RATIO(7) | COREM1_RATIO(7) | PERIPH_RATIO(7) | ATB_RATIO(7) | PCLK_DBG_RATIO(7) | APLL_RATIO(7) | CORE2_RATIO(7);

	clrsetbits_le32(&clk->div_cpu0, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_cpu0) & DIV_STAT_CPU0_CHANGING)
		continue;
	
	set = COPY_RATIO(4) | HPM_RATIO(0);
	clr = COPY_RATIO(7) | HPM_RATIO(7);
	clrsetbits_le32(&clk->div_cpu1, clr, set);
	while (readl(&clk->div_stat_cpu1) & DIV_STAT_CPU1_CHANGING)
		continue;

	/* Set APLL to 1000MHz */
	writel (0x00803800, &clk->apll_con1);
	clr_pll_con0 = SDIV(7) | PDIV(63) | MDIV(1023) | FSEL(1);
	set = SDIV(0) | PDIV(3) | MDIV(125) | FSEL(0) | PLL_ENABLE(1);

	clrsetbits_le32(&clk->apll_con0, clr_pll_con0, set);

	/* Wait for PLL to be locked */
	while (!(readl(&clk->apll_con0) & PLL_LOCKED_BIT))
		continue;

	/* Set MPLL to 800MHz */
	writel (0x00803800, &clk->mpll_con1);
	set = SDIV(0) | PDIV(3) | MDIV(100) | FSEL(0) | PLL_ENABLE(1);

	clrsetbits_le32(&clk->mpll_con0, clr_pll_con0, set);

	/* Wait for PLL to be locked */
	while (!(readl(&clk->mpll_con0) & PLL_LOCKED_BIT))
		continue;

	// set EPLL
	writel (0x80, &clk->epll_con2);
	writel (0x66010000, &clk->epll_con1);
	writel (EPLL_CON0_VAL, &clk->epll_con0);
	while (!(readl(&clk->epll_con0) & PLL_LOCKED_BIT))
		continue;

	// set VPLL
	writel (0x0080, &clk->vpll_con2);
	writel (0x66010000, &clk->vpll_con1);
	writel (VPLL_CON0_VAL, &clk->vpll_con0);
	while (!(readl(&clk->vpll_con0) & PLL_LOCKED_BIT))
		continue;

	clr_src_cpu = MUX_APLL_SEL(1) | MUX_CORE_SEL(1) |
		      MUX_HPM_SEL(1) | MUX_MPLL_USER_SEL_C(1);
	set = MUX_APLL_SEL(1) | MUX_CORE_SEL(0) | MUX_HPM_SEL(0) | MUX_MPLL_USER_SEL_C(1);

	clrsetbits_le32(&clk->src_cpu, clr_src_cpu, set);

	/* Wait for mux change */
	while (readl(&clk->mux_stat_cpu) & MUX_STAT_CPU_CHANGING)
		continue;
	

#if 1 /* USART0 Initialize add by zhuchengzhi 2018-06-02  TODO */
	/* CLK_SRC_PERIL0 */
	clr = UART0_SEL(15) | UART1_SEL(15) | UART2_SEL(15) |
	      UART3_SEL(15) | UART4_SEL(15);
	/*
	 * Set CLK_SRC_PERIL0 clocks src to MPLL
	 * src values: 0(XXTI); 1(XusbXTI); 2(SCLK_HDMI24M); 3(SCLK_USBPHY0);
	 *             5(SCLK_HDMIPHY); 6(SCLK_MPLL_USER_T); 7(SCLK_EPLL);
	 *             8(SCLK_VPLL)
	 *
	 * Set all to SCLK_MPLL_USER_T
	 */
	set = UART0_SEL(6) | UART1_SEL(6) | UART2_SEL(6) | UART3_SEL(6) | UART4_SEL(6);

	clrsetbits_le32(&clk->src_peril0, clr, set);

	/* CLK_DIV_PERIL0 */
	clr = UART0_RATIO(15) | UART1_RATIO(15) | UART2_RATIO(15) | UART3_RATIO(15) | UART4_RATIO(15);
	/*
	 * For MOUTuart0-4: 800MHz
	 *
	 * SCLK_UARTx = MOUTuartX / (ratio + 1) = 100 (7)
	*/
	set = UART0_RATIO(7) | UART1_RATIO(7) | UART2_RATIO(7) | UART3_RATIO(7) | UART4_RATIO(7);

	clrsetbits_le32(&clk->div_peril0, clr, set);

	while (readl(&clk->div_stat_peril0) & DIV_STAT_PERIL0_CHANGING)
		continue;

#endif

	// set src_dmc
	clr_src_dmc = MUX_C2C_SEL(1) | MUX_DMC_BUS_SEL(1) |
		      MUX_DPHY_SEL(1) | MUX_MPLL_SEL(1) |
		      MUX_PWI_SEL(15) | MUX_G2D_ACP0_SEL(1) |
		      MUX_G2D_ACP1_SEL(1) | MUX_G2D_ACP_SEL(1);
	
	set = MUX_C2C_SEL(0) | MUX_DMC_BUS_SEL(0) | MUX_DPHY_SEL(0) | MUX_MPLL_SEL(1) | MUX_PWI_SEL(1) | MUX_G2D_ACP0_SEL(0) | MUX_G2D_ACP1_SEL(0) | MUX_G2D_ACP_SEL(0);

	clrsetbits_le32(&clk->src_dmc, clr_src_dmc, set);

	/* Wait for mux change */
	while (readl(&clk->mux_stat_dmc) & MUX_STAT_DMC_CHANGING)
		continue;

	writel (0x0110, &clk->src_top0);
	while (readl(&clk->mux_stat_top0) != 0x11111221)
		continue;

	writel (0x00011000, &clk->src_top1);
	while (readl(&clk->mux_stat_top1) != 0x01122110)
		continue;

	addr = (unsigned int *)CHIP_BASE;
	value = readl(addr);

	if (((value >> 8) & 0x3) != 2)
		return;
	
	addr = (unsigned int *)(LANDROVER_POWER_BASE+C2C_CTRL_OFFSET);
	value = readl(addr);
	if ((value & 0x1) != 0)
		return;

	addr = (unsigned int *)(APB_DMC_0_BASE+DMC_PHYCONTROL0);
	writel (0x7F10100A, addr);

	addr = (unsigned int *)(APB_DMC_0_BASE+DMC_PHYCONTROL1);
	writel (0xE0000084, addr);

	addr = (unsigned int *)(APB_DMC_0_BASE+DMC_PHYCONTROL0);
	writel (0x7F10100B, addr);
	while (!(readl(APB_DMC_0_BASE+DMC_PHYSTATUS) & (1<<2)))
		continue;

	addr = (unsigned int *)(APB_DMC_0_BASE+DMC_PHYCONTROL1);
	writel (0x0000008C, addr);
	writel (0x00000084, addr);
	while (!(readl(APB_DMC_0_BASE+DMC_PHYSTATUS) & (1<<2)))
		continue;
	
	addr = (unsigned int *)(APB_DMC_1_BASE+DMC_PHYCONTROL0);
	writel (0x7F10100A, addr);

	addr = (unsigned int *)(APB_DMC_1_BASE+DMC_PHYCONTROL1);
	writel (0xE0000084, addr);

	addr = (unsigned int *)(APB_DMC_1_BASE+DMC_PHYCONTROL0);
	writel (0x7F10100B, addr);
	while (!(readl(APB_DMC_1_BASE+DMC_PHYSTATUS) & (1<<2)))
		continue;

	addr = (unsigned int *)(APB_DMC_1_BASE+DMC_PHYCONTROL1);
	writel (0x0000008C, addr);
	writel (0x00000084, addr);
	while (!(readl(APB_DMC_1_BASE+DMC_PHYSTATUS) & (1<<2)))
		continue;

	
	addr = (unsigned int *)(APB_DMC_0_BASE+DMC_CONCONTROL);
	writel (0x0FFF30FA, addr);

	addr = (unsigned int *)(APB_DMC_1_BASE+DMC_CONCONTROL);
	writel (0x0FFF30FA, addr);

	addr = (unsigned int *)(APB_DMC_0_BASE+DMC_MEMCONTROL);
	writel (0x00202533, addr);

	addr = (unsigned int *)(APB_DMC_1_BASE+DMC_MEMCONTROL);
	writel (0x00202533, addr);

	// eMMC clock configuration
	set_mmc_clk(4, 0);

	return;
}


#else  /*old  */ 

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include "common_setup.h"
#include "exynos4_setup.h"

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
void system_clock_init(void)
{
	struct exynos4_clock *clk =
			(struct exynos4_clock *)samsung_get_base_clock();

	writel(CLK_SRC_CPU_VAL, &clk->src_cpu);

	sdelay(0x10000);

	writel(CLK_SRC_TOP0_VAL, &clk->src_top0);
	writel(CLK_SRC_TOP1_VAL, &clk->src_top1);
	writel(CLK_SRC_DMC_VAL, &clk->src_dmc);
	writel(CLK_SRC_LEFTBUS_VAL, &clk->src_leftbus);
	writel(CLK_SRC_RIGHTBUS_VAL, &clk->src_rightbus);
	writel(CLK_SRC_FSYS_VAL, &clk->src_fsys);
	writel(CLK_SRC_PERIL0_VAL, &clk->src_peril0);
	writel(CLK_SRC_CAM_VAL, &clk->src_cam);
	writel(CLK_SRC_MFC_VAL, &clk->src_mfc);
	writel(CLK_SRC_G3D_VAL, &clk->src_g3d);
	writel(CLK_SRC_LCD0_VAL, &clk->src_lcd0);

	sdelay(0x10000);

	writel(CLK_DIV_CPU0_VAL, &clk->div_cpu0);
	writel(CLK_DIV_CPU1_VAL, &clk->div_cpu1);
	writel(CLK_DIV_DMC0_VAL, &clk->div_dmc0);
	writel(CLK_DIV_DMC1_VAL, &clk->div_dmc1);
	writel(CLK_DIV_LEFTBUS_VAL, &clk->div_leftbus);
	writel(CLK_DIV_RIGHTBUS_VAL, &clk->div_rightbus);
	writel(CLK_DIV_TOP_VAL, &clk->div_top);
	writel(CLK_DIV_FSYS1_VAL, &clk->div_fsys1);
	writel(CLK_DIV_FSYS2_VAL, &clk->div_fsys2);
	writel(CLK_DIV_FSYS3_VAL, &clk->div_fsys3);
	writel(CLK_DIV_PERIL0_VAL, &clk->div_peril0);
	writel(CLK_DIV_CAM_VAL, &clk->div_cam);
	writel(CLK_DIV_MFC_VAL, &clk->div_mfc);
	writel(CLK_DIV_G3D_VAL, &clk->div_g3d);
	writel(CLK_DIV_LCD0_VAL, &clk->div_lcd0);

	/* Set PLL locktime */
	writel(PLL_LOCKTIME, &clk->apll_lock);
	writel(PLL_LOCKTIME, &clk->mpll_lock);
	writel(PLL_LOCKTIME, &clk->epll_lock);
	writel(PLL_LOCKTIME, &clk->vpll_lock);

	writel(APLL_CON1_VAL, &clk->apll_con1);
	writel(APLL_CON0_VAL, &clk->apll_con0);
	writel(MPLL_CON1_VAL, &clk->mpll_con1);
	writel(MPLL_CON0_VAL, &clk->mpll_con0);
	writel(EPLL_CON1_VAL, &clk->epll_con1);
	writel(EPLL_CON0_VAL, &clk->epll_con0);
	writel(VPLL_CON1_VAL, &clk->vpll_con1);
	writel(VPLL_CON0_VAL, &clk->vpll_con0);

	sdelay(0x30000);
}
#endif
