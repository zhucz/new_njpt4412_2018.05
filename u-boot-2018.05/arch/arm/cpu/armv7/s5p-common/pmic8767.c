#include <common.h>
#include <config.h>
#include <debug_uart.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dmc.h>
#include <asm/arch/power.h>
#include <asm/arch/tzpc.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/system.h>
#include <asm/armv7.h>

#include "asm/io.h"

#include <asm/io.h>
#include "pmic8767.h"

extern void lowlevel_init_pmic8767(unsigned char Address,unsigned char *Val,int flag);

#define CONFIG_PM_VDD_ARM       1.2
#define CONFIG_PM_VDD_INT       1.0
#define CONFIG_PM_VDD_G3D       1.1
#define CONFIG_PM_VDD_MIF       1.1
#define CONFIG_PM_VDD_LDO14     1.8


typedef enum{	PMIC_BUCK1=0,	PMIC_BUCK2,	PMIC_BUCK3,	PMIC_BUCK4,	PMIC_LDO14,	PMIC_LDO10,}PMIC_RegNum;


#define CALC_S5M8767_VOLT1(x)  ( (x<600) ? 0 : ((x-600)/6.25) )
#define CALC_S5M8767_VOLT2(x)  ( (x<650) ? 0 : ((x-650)/6.25) )

void I2C_S5M8767_VolSetting(PMIC_RegNum eRegNum, u8 ucVolLevel, u8 ucEnable)
{
	u8 reg_addr;
	u8 reg_bitmask, vol_level;


	reg_bitmask = 0xFF;
	if(eRegNum == 0)
	{
		reg_addr = 0x33;
	}
	else if(eRegNum == 1)
	{
		reg_addr = 0x35;
	}
	else if(eRegNum == 2)
	{
		reg_addr = 0x3E;
	}
	else if(eRegNum == 3)
	{
		reg_addr = 0x47;
	}
	else if(eRegNum == 4)
	{
		reg_addr = 0x48;
	}
	/* add by cym 20130315 */
	else if(7 == eRegNum)
	{
		reg_addr = 0x59;
		ucVolLevel = 0x40;	// 1.55v
	}
	/* end add */
	else
		while(1);

	vol_level = ucVolLevel&reg_bitmask;
	lowlevel_init_pmic8767(reg_addr,&vol_level,1);

}


void vdd_pmic8767_init(void)
{
	float vdd_arm, vdd_int, vdd_g3d;
	float vdd_mif;
//	u8 read_data;

	vdd_arm = CONFIG_PM_VDD_ARM;
	vdd_int = CONFIG_PM_VDD_INT;
	vdd_g3d = CONFIG_PM_VDD_G3D;
	vdd_mif = CONFIG_PM_VDD_MIF;

	I2C_S5M8767_VolSetting(PMIC_BUCK1, CALC_S5M8767_VOLT2(vdd_mif * 1000), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK2, CALC_S5M8767_VOLT1(vdd_arm * 1000), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK3, CALC_S5M8767_VOLT1(vdd_int * 1000), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK4, CALC_S5M8767_VOLT1(vdd_g3d * 1000), 1);

	/* add by cym 20130315 */
	//I2C_S5M8767_VolSetting(6, CALC_S5M8767_VOLT1(1.5 * 1000), 1);

	//set Buck8 to 1.5v, because LDO2's out  decide by Buck8
	I2C_S5M8767_VolSetting(7, CALC_S5M8767_VOLT1(1.55 * 1000), 1);

	/* end add */

	printf("vdd_pmic8767_init ---- \n");
	return;
}

void PMIC8767_InitIp(void)
{
	u8 id;
	u8 reg_5;

	exynos_pinmux_config(PERIPH_ID_I2C1, PINMUX_FLAG_NONE);

	lowlevel_init_pmic8767(0,&id,0);
	lowlevel_init_pmic8767(0x05,&reg_5,0);

	printf("S5M8767(VER5.0) reg_5 = %#x \n",reg_5);

	if(id == 0x3 || (0x5 == id) || (21 == id))	{
		printf("S5M8767(VER5.0) id = %#x \n",id);
    }else{
		printf("Not Found S5M8767 \n"); 
	}  
	    

	vdd_pmic8767_init( );

	return;
}


